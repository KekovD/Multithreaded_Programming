#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H

#include "headers.h"
#include "room.h"
#include "room_command.h"
#include <boost/smart_ptr.hpp>
#include <boost/asio/steady_timer.hpp>

using json = nlohmann::json;

class SocketConnection : public boost::enable_shared_from_this<SocketConnection> {
public:
    explicit SocketConnection(ip::tcp::socket&& socket);
    ~SocketConnection();

    void InitializeWebsocket();
    void TransmitData(const std::string& payload);
    void ProcessClientCommand(const RoomCommand& cmd);
    std::string GetUserIdentifier() const;

private:
    websocket::stream<tcp_stream> websocket_stream_;
    flat_buffer data_buffer_;
    boost::shared_ptr<Room> associated_room_;
    std::string user_identity_;
    steady_timer heartbeat_timer_;
    std::atomic<bool> pong_received_{true};

    void TerminateConnection(const std::string& reason, websocket::close_code close_code);
    void ProcessInitialHandshake();
    void UpdateRoomListing();
    void ConfigureHeartbeat();
    void PrepareSessionData();
    void MaintainConnection();
};

#endif // SOCKET_CONNECTION_H