#include "../include/room.h"
#include "../include/rooms_singleton.h"
#include "../include/socket_connection.h"
#include <boost/thread/lock_guard.hpp>
#include <chrono>

Room::Room(std::string room_name)
    : room_identifier_(std::move(room_name)) {}

void Room::AddConnection(boost::weak_ptr<SocketConnection> connection) {
    boost::lock_guard guard(room_mutex_);
    connections_.emplace_back(std::move(connection));
}

void Room::RemoveConnection(const boost::weak_ptr<SocketConnection> &connection) {
    boost::lock_guard guard(room_mutex_);

    std::erase_if(
        connections_,
        [&](const auto& conn) {
            auto locked = connection.lock();
            auto current = conn.lock();
            return !current || (locked && current == locked);
        }
    );

    if (connections_.empty()) {
        post([this] {
            RoomsSingleton::GetInstance()->UnregisterRoom(room_identifier_);
        });
    }
}

void Room::DistributeMessage(const std::string& sender, const std::string& content) {
    auto formatted = FormatMessage(sender, content);

    boost::lock_guard guard(room_mutex_);
    message_history_.emplace_back(formatted);

    for (auto& conn : connections_) {
        post(
            delivery_pool_,
            [formatted, conn] {
                ProcessMessageDelivery(formatted, conn);
            }
        );
    }
}

std::vector<std::string> Room::GetMessageHistory() const {
    boost::lock_guard guard(room_mutex_);
    return message_history_;
}

std::vector<std::string> Room::GetActiveUsers() const {
    std::vector<std::string> users;
    boost::lock_guard guard(room_mutex_);

    for (const auto& conn : connections_) {
        if (const auto connection = conn.lock()) {
            users.emplace_back(connection->GetUserIdentifier());
        }
    }

    return users;
}

auto Room::ProcessMessageDelivery(
    const std::string &formatted_message,
    const boost::weak_ptr<SocketConnection> &connection
) -> void {
    if (const auto conn = connection.lock()) {
        conn->TransmitData(formatted_message);
    }
}

std::string Room::FormatMessage(
    const std::string& username,
    const std::string& message
) {
    namespace chrono = std::chrono;
    auto now = chrono::floor<chrono::seconds>(chrono::system_clock::now());
    return std::format("[{:%T}]<{}> {}", now, username, message);
}