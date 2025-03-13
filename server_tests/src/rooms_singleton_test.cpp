#include "gtest/gtest.h"
#include "../../server/include/rooms_singleton.h"
#include "../../server/include/connection_acceptor.h"
#include <vector>
#include <stdexcept>

class RoomsSingletonTest : public ::testing::Test {
protected:
    RoomsSingletonTest() = default;

    void SetUp() override {
        rooms = RoomsSingleton::GetInstance();
        rooms->PurgeAllRooms();
    }

    boost::shared_ptr<RoomsSingleton> rooms = nullptr;
};

TEST_F(RoomsSingletonTest, SingletonInstance) {
    const auto instance = RoomsSingleton::GetInstance();
    EXPECT_EQ(rooms, instance);
}

TEST_F(RoomsSingletonTest, AddRoomGetRoom) {
    auto roomName = "room";
    const auto room = boost::make_shared<Room>(roomName);

    rooms->RegisterRoom(roomName, room);
    const auto getRoom = rooms->FetchRoom(roomName);

    EXPECT_EQ(room, getRoom);
}

TEST_F(RoomsSingletonTest, AddRoomRemoveRoom) {
    auto roomName = "room";
    const auto room = boost::make_shared<Room>(roomName);

    rooms->RegisterRoom(roomName, room);
    rooms->UnregisterRoom(roomName);

    ASSERT_THROW(rooms->FetchRoom(roomName), std::out_of_range);
}

TEST_F(RoomsSingletonTest, RemoveNonExistentRoom) {
    EXPECT_NO_THROW(rooms->UnregisterRoom("non_existent_room"));
}

TEST_F(RoomsSingletonTest, GetNonExistentRoom) {
    EXPECT_THROW(rooms->FetchRoom("unknown_room"), std::out_of_range);
}

TEST_F(RoomsSingletonTest, RoomNamesAfterRemoval) {
    rooms->RegisterRoom("room1", boost::make_shared<Room>("room1"));
    rooms->RegisterRoom("room2", boost::make_shared<Room>("room2"));
    rooms->UnregisterRoom("room1");

    const auto names = rooms->GetAllRoomNames();
    ASSERT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], "room2");
}

TEST_F(RoomsSingletonTest, AddRoomsGetRoomNames) {
    const auto room1 = boost::make_shared<Room>("room1");
    const auto room2 = boost::make_shared<Room>("room2");

    rooms->RegisterRoom("room1", room1);
    rooms->RegisterRoom("room2", room2);

    auto roomNames = rooms->GetAllRoomNames();
    EXPECT_EQ(roomNames.size(), 2);
    EXPECT_NE(std::ranges::find(roomNames, "room1"), roomNames.end());
    EXPECT_NE(std::ranges::find(roomNames, "room2"), roomNames.end());
}

TEST_F(RoomsSingletonTest, OverwriteExistingRoom) {
    const auto room1 = boost::make_shared<Room>("room");
    const auto room2 = boost::make_shared<Room>("room");

    rooms->RegisterRoom("room", room1);
    EXPECT_THROW(rooms->RegisterRoom("room", room2), std::runtime_error);
}