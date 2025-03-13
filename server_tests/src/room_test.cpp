#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../../server/include/room.h"
#include "../../server/include/rooms_singleton.h"
#include "mock_socket_connection.cpp"
#include <boost/asio.hpp>
#include <chrono>

using namespace testing;
using namespace std::chrono_literals;

class RoomTest : public Test {
protected:
    void SetUp() override {
        rooms = RoomsSingleton::GetInstance();
        rooms->UnregisterRoom("test_room");
    }

    boost::shared_ptr<RoomsSingleton> rooms;
    std::shared_ptr<MockSocketConnection> mock_socket = std::make_shared<MockSocketConnection>();
};

class TestSocketConnection : public SocketConnection {
public:
    explicit TestSocketConnection(ip::tcp::socket&& socket, std::string user_id)
        : SocketConnection(std::move(socket)), user_id_(std::move(user_id)) {}

    TestSocketConnection(std::string user_id)
        : SocketConnection(ip::tcp::socket(MockSocketConnection::get_io_context())),
          user_id_(std::move(user_id)) {}

    std::string GetUserIdentifier() const {
        return user_id_;
    }

private:
    std::string user_id_;
};

TEST_F(RoomTest, RoomCreationAndRegistration) {
    const auto room = boost::make_shared<Room>("test_lounge");

    rooms->RegisterRoom("test_lounge", room);
    const auto retrieved = rooms->FetchRoom("test_lounge");

    EXPECT_EQ(room, retrieved);
    EXPECT_EQ(retrieved->GetActiveUsers().size(), 0);
}

TEST_F(RoomTest, ConnectionLifecycle) {
    Room chat_room("general_chat");

    const auto mock_socket = boost::make_shared<MockSocketConnection>();
    const boost::weak_ptr<SocketConnection> weak_conn(mock_socket);

    chat_room.AddConnection(weak_conn);
    EXPECT_THAT(chat_room.GetActiveUsers(), SizeIs(1));

    chat_room.RemoveConnection(weak_conn);
    EXPECT_THAT(chat_room.GetActiveUsers(), IsEmpty());
}

TEST_F(RoomTest, UserListManagement) {
    Room vip_room("exclusive");
    const auto user1 = boost::make_shared<MockSocketConnection>();
    const auto user2 = boost::make_shared<MockSocketConnection>();

    vip_room.AddConnection(boost::static_pointer_cast<SocketConnection>(user1));
    vip_room.AddConnection(boost::static_pointer_cast<SocketConnection>(user2));

    const auto users_number = vip_room.GetActiveUsers().size();
    EXPECT_THAT(users_number, 2);

    vip_room.RemoveConnection(user1);
    EXPECT_THAT(vip_room.GetActiveUsers().size(), 1);
}