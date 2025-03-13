#include "../include/connection_acceptor.h"
#include "../include/socket_connection.h"

ConnectionAcceptor::ConnectionAcceptor(
    io_context& io_context,
    const ip::tcp::endpoint& endpoint,
    const size_t thread_pool_size
) :
    io_context_(io_context),
    acceptor_(io_context),
    thread_pool_size_(thread_pool_size)
{
    InitAcceptor();
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

ConnectionAcceptor::~ConnectionAcceptor() {
    Stop();
}

void ConnectionAcceptor::Start() {
    is_accepting_ = true;
    AsyncAccept();

    for(size_t i = 0; i < thread_pool_size_; ++i) {
        worker_threads_.emplace_back(&ConnectionAcceptor::WorkerThread, this);
    }
}

void ConnectionAcceptor::Stop() {
    is_accepting_ = false;
    acceptor_.close();

    for(auto& thread : worker_threads_) {
        if(thread.joinable()) {
            thread.join();
        }
    }
}

void ConnectionAcceptor::InitAcceptor() {
    acceptor_.open(ip::tcp::v4());
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
}

void ConnectionAcceptor::AsyncAccept() {
    if(!is_accepting_) return;

    auto socket = boost::make_shared<ip::tcp::socket>(io_context_);

    acceptor_.async_accept(
        *socket,
        [this, socket](const boost::system::error_code& error) {
            HandleAccept(socket, error);
        }
    );
}

void ConnectionAcceptor::HandleAccept(
    const boost::shared_ptr<ip::tcp::socket>& socket,
    const boost::system::error_code& error
) {
    if(!error) {
        const auto connection = boost::make_shared<SocketConnection>(std::move(*socket));
        ProcessConnection(connection);
    }

    if(is_accepting_) {
        AsyncAccept();
    }
}

void ConnectionAcceptor::ProcessConnection(const boost::shared_ptr<SocketConnection>& connection) const {
    post(
        io_context_,
        [connection] {
            connection->InitializeWebsocket();
        }
    );
}

void ConnectionAcceptor::WorkerThread() const {
    while(is_accepting_) {
        try {
            io_context_.run();
        }
        catch(const std::exception& e) {
            std::cerr << "Worker thread exception: " << e.what() << std::endl;
        }
    }
}
