#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../../server/include/socket_connection.h"
#include <boost/asio/ip/tcp.hpp>

class MockSocketConnection final : public SocketConnection {
    public:
    static io_context& get_io_context() {
        static io_context io;
        return io;
    }

    explicit MockSocketConnection(ip::tcp::socket&& socket) : SocketConnection(std::move(socket)) {}

    MockSocketConnection() : SocketConnection(ip::tcp::socket(get_io_context())) {}

    MOCK_METHOD(void, InitializeWebsocket, ());
    MOCK_METHOD(void, TransmitData, (const std::string&));
    MOCK_METHOD(std::string, GetUserIdentifier, (), (const));
};
