#ifndef CONNECTION_ACCEPTOR_H
#define CONNECTION_ACCEPTOR_H

#include "headers.h"
#include "socket_connection.h"
#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <vector>

class SocketConnection;

class ConnectionAcceptor {
public:
    using ConnectionFactory = std::function<boost::shared_ptr<SocketConnection>(ip::tcp::socket)>;

    ConnectionAcceptor(
        io_context& io_context,
        const ip::tcp::endpoint& endpoint,
        size_t thread_pool_size = boost::thread::hardware_concurrency()
    );

    ~ConnectionAcceptor();

    void Start();
    void Stop();

private:
    void InitAcceptor();
    void AsyncAccept();
    void HandleAccept(
        const boost::shared_ptr<ip::tcp::socket>& socket,
        const boost::system::error_code& error
    );
    void ProcessConnection(const boost::shared_ptr<SocketConnection>& connection) const;
    void WorkerThread() const;

    io_context& io_context_;
    ip::tcp::acceptor acceptor_;
    boost::atomic<bool> is_accepting_{false};
    std::vector<std::thread> worker_threads_;
    size_t thread_pool_size_;
};

#endif // CONNECTION_ACCEPTOR_H
