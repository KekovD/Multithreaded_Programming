#ifndef ROOM_H
#define ROOM_H

#include <vector>
#include <string>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/mutex.hpp>

class SocketConnection;

class Room {
public:
    explicit Room(std::string room_name);

    void AddConnection(boost::weak_ptr<SocketConnection> connection);
    void RemoveConnection(const boost::weak_ptr<SocketConnection> &connection);
    void DistributeMessage(const std::string& content);

    std::vector<std::string> GetMessageHistory() const;
    std::vector<std::string> GetActiveUsers() const;

private:
    static void ProcessMessageDelivery(
        const std::string& formatted_message,
        const boost::weak_ptr<SocketConnection> &connection
    );

    mutable boost::mutex room_mutex_;
    std::vector<boost::weak_ptr<SocketConnection>> connections_;
    std::vector<std::string> message_history_;
    std::string room_identifier_;
    boost::asio::thread_pool delivery_pool_{4};
};

#endif // ROOM_H