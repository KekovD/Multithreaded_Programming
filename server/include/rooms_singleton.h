#ifndef ROOMSSINGLETON_H
#define ROOMSSINGLETON_H

#include <vector>
#include <string>
#include <unordered_map>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "room.h"

class RoomsSingleton {
    static boost::shared_ptr<RoomsSingleton> instance_;
    static boost::mutex instance_mutex_;

    std::unordered_map<std::string, boost::shared_ptr<Room>> rooms_;
    mutable boost::shared_mutex rooms_mutex_;

    explicit RoomsSingleton() = default;

    static void ValidateName(const std::string& name);

public:
    RoomsSingleton(const RoomsSingleton&) = delete;
    RoomsSingleton& operator=(const RoomsSingleton&) = delete;

    static boost::shared_ptr<RoomsSingleton> GetInstance();

    void RegisterRoom(const std::string& name, boost::shared_ptr<Room> room);
    void UnregisterRoom(const std::string& name);
    boost::shared_ptr<Room> FetchRoom(const std::string& name) const;

    std::vector<std::string> GetAllRoomNames() const;
    void PurgeAllRooms();
};

#endif // ROOMSSINGLETON_H
