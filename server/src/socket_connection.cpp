#include "../include/socket_connection.h"
#include "../include/rooms_singleton.h"

SocketConnection::SocketConnection(ip::tcp::socket&& socket)
    : websocket_stream_(std::move(socket)),
      heartbeat_timer_(websocket_stream_.get_executor()) {}

SocketConnection::~SocketConnection() {
    if (const auto room = associated_room_) {
        room->RemoveConnection(boost::weak_ptr(shared_from_this()));
    }
}

void SocketConnection::InitializeWebsocket() {
    websocket_stream_.async_accept(
        [self = shared_from_this()](const error_code& ec) {
            if (!ec) self->ProcessInitialHandshake();
        });
}

void SocketConnection::TransmitData(const std::string& payload) {
    websocket_stream_.async_write(
        buffer(payload),
        [self = shared_from_this()](error_code, auto) {}
        );
}

void SocketConnection::ProcessInitialHandshake() {
    websocket_stream_.control_callback(
        [self = shared_from_this()](const websocket::frame_type ft, auto&&) {
            if (ft == websocket::frame_type::pong) self->pong_received_ = true;
        });

    UpdateRoomListing();
}

void SocketConnection::UpdateRoomListing() {
    const auto rooms = RoomsSingleton::GetInstance();
    const json response{{"rooms", rooms->GetAllRoomNames()}};

    TransmitData(response.dump());
    PrepareSessionData();
}

void SocketConnection::PrepareSessionData() {
    websocket_stream_.async_read(data_buffer_,
        [self = shared_from_this()](error_code ec, auto) {
            if (!ec) self->ProcessClientCommand(
                json::parse(buffers_to_string(self->data_buffer_.data()))
            );
        });
}

void SocketConnection::ProcessClientCommand(const RoomCommand& cmd) {
    user_identity_ = cmd.userName;
    const auto rooms = RoomsSingleton::GetInstance();

    if (cmd.operation == "create") {
        associated_room_ = boost::make_shared<Room>(cmd.roomName);
        rooms->RegisterRoom(cmd.roomName, associated_room_);
    } else if (cmd.operation == "join") {
        associated_room_ = rooms->FetchRoom(cmd.roomName);
    }

    if (associated_room_) {
        associated_room_->AddConnection(shared_from_this());
        MaintainConnection();
    }
}

void SocketConnection::MaintainConnection() {
    ConfigureHeartbeat();
    ReadNextMessage();
}

void SocketConnection::ReadNextMessage() {
    data_buffer_.consume(data_buffer_.size());

    websocket_stream_.async_read(
        data_buffer_,
        [self = shared_from_this()](const error_code &ec, std::size_t) {
            if (ec) {
                std::cerr << "WebSocket read error: " << ec.message() << std::endl;
                return;
            }

            try {
                self->associated_room_->DistributeMessage(buffers_to_string(self->data_buffer_.data()));

                self->ReadNextMessage();
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing message: " << e.what() << std::endl;
            }
        }
    );
}

void SocketConnection::ConfigureHeartbeat() {
    heartbeat_timer_.cancel();

    heartbeat_timer_.expires_after(std::chrono::seconds(5));
    heartbeat_timer_.async_wait(
        [self = shared_from_this()](const error_code &ec) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (!ec && !self->pong_received_) {
                std::cout << "Heartbeat failure detected for user: " << self->user_identity_ << std::endl;
                self->TerminateConnection("Heartbeat failure", websocket::close_code::internal_error);
                return;
            }

            self->pong_received_ = false;

            try {
                self->websocket_stream_.async_ping(
                    "",
                    [](auto){}
                );

                self->ConfigureHeartbeat();
            }
            catch (const std::exception& e) {
                std::cerr << "Error in heartbeat ping: " << e.what() << std::endl;
            }
        }
    );
}

std::string SocketConnection::GetUserIdentifier() const {
    return user_identity_;
}

void SocketConnection::TerminateConnection(const std::string &reason, const websocket::close_code close_code) {
    const websocket::close_reason cr{
        close_code,
        reason
    };
    websocket_stream_.async_close(cr, [](auto){});
}