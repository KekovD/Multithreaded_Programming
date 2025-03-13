#include "../include/rooms_singleton.h"

boost::shared_ptr<RoomsSingleton> RoomsSingleton::instance_;
boost::mutex RoomsSingleton::instance_mutex_;

boost::shared_ptr<RoomsSingleton> RoomsSingleton::GetInstance() {
    boost::lock_guard lock(instance_mutex_);
    if (!instance_) {
        instance_.reset(new RoomsSingleton());
    }
    return instance_;
}

void RoomsSingleton::ValidateName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Room name cannot be empty");
    }
}

void RoomsSingleton::RegisterRoom(const std::string& name, boost::shared_ptr<Room> room) {
    ValidateName(name);
    boost::unique_lock lock(rooms_mutex_);

    if (rooms_.contains(name)) {
        throw std::runtime_error("Room already exists: " + name);
    }
    rooms_.emplace(name, std::move(room));
}

void RoomsSingleton::UnregisterRoom(const std::string& name) {
    ValidateName(name);
    boost::unique_lock lock(rooms_mutex_);
    rooms_.erase(name);
}

boost::shared_ptr<Room> RoomsSingleton::FetchRoom(const std::string& name) const {
    ValidateName(name);
    boost::shared_lock lock(rooms_mutex_);

    const auto it = rooms_.find(name);
    if (it == rooms_.end()) {
        throw std::out_of_range("Room not found: " + name);
    }
    return it->second;
}

std::vector<std::string> RoomsSingleton::GetAllRoomNames() const {
    boost::shared_lock lock(rooms_mutex_);

    std::vector<std::string> names;
    names.reserve(rooms_.size());
    for (const auto& pair : rooms_) {
        names.push_back(pair.first);
    }
    return names;
}

void RoomsSingleton::PurgeAllRooms() {
    boost::unique_lock lock(rooms_mutex_);
    rooms_.clear();
}