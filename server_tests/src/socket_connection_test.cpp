#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../../server/include/socket_connection.h"
#include "../../server/include/rooms_singleton.h"
#include "mock_socket_connection.cpp"

class SocketConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        RoomsSingleton::GetInstance()->PurgeAllRooms();
    }
};

TEST_F(SocketConnectionTest, CreateRoomCommand) {
    const auto connection = boost::make_shared<MockSocketConnection>();

    RoomCommand cmd;
    cmd.operation = "create";
    cmd.roomName = "test_room";
    cmd.userName = "test_user";

    EXPECT_CALL(*connection, GetUserIdentifier())
        .WillRepeatedly(::testing::Return("test_user"));

    connection->ProcessClientCommand(cmd);

    const auto room = RoomsSingleton::GetInstance()->FetchRoom("test_room");
    ASSERT_NE(room, nullptr);

    const auto users = room->GetActiveUsers();
    ASSERT_EQ(users.size(), 1);
    EXPECT_EQ(users[0], "test_user");
}

TEST_F(SocketConnectionTest, JoinRoomCommand) {
    const auto room = boost::make_shared<Room>("existing_room");
    RoomsSingleton::GetInstance()->RegisterRoom("existing_room", room);

    const auto connection = boost::make_shared<MockSocketConnection>();

    RoomCommand cmd;
    cmd.operation = "join";
    cmd.roomName = "existing_room";
    cmd.userName = "joining_user";

    EXPECT_CALL(*connection, GetUserIdentifier())
        .WillRepeatedly(::testing::Return("joining_user"));

    connection->ProcessClientCommand(cmd);

    const auto users = room->GetActiveUsers();
    ASSERT_EQ(users.size(), 1);
    EXPECT_EQ(users[0], "joining_user");
}

TEST_F(SocketConnectionTest, BroadcastMessage) {
    const auto room = boost::make_shared<Room>("chat_room");
    RoomsSingleton::GetInstance()->RegisterRoom("chat_room", room);

    const auto sender = boost::make_shared<MockSocketConnection>();
    const auto receiver = boost::make_shared<MockSocketConnection>();

    EXPECT_CALL(*sender, GetUserIdentifier())
        .WillRepeatedly(::testing::Return("sender_user"));
    EXPECT_CALL(*receiver, GetUserIdentifier())
        .WillRepeatedly(::testing::Return("receiver_user"));

    room->AddConnection(sender);
    room->AddConnection(receiver);

    room->DistributeMessage("sender_user", "Hello, world!");

    auto messages = room->GetMessageHistory();
    ASSERT_EQ(messages.size(), 1);
    EXPECT_NE(messages[0].find("Hello, world!"), std::string::npos);
}
