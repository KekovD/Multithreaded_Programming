#include "../include/connection_acceptor.h"
#include "../include/server_controller.h"
#include <csignal>

boost::atomic<bool> ServerController::g_force_exit{false};

int ServerController::RunService(int argc, char** argv) {
    if (!ValidateArguments(argc, argv)) return 1;

    try {
        io_context io_ctx;
        auto work = make_work_guard(io_ctx);

        SetupSignalHandling();
        InitializeNetworkComponents(io_ctx, argv[1]);

        std::cerr << "Server started on port " << argv[1] << std::endl;
        std::cerr.flush();

        std::vector<std::thread> threads;
        constexpr size_t num_threads = 2;
        for(size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&io_ctx] {
                io_ctx.run();
            });
        }

        while(!g_force_exit.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        io_ctx.stop();
        for(auto& t : threads) if(t.joinable()) t.join();

        std::cerr << "\nServer stopped" << std::endl;
        std::cerr.flush();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}

void ServerController::ThreadPoolWrapper::WaitForCompletion() {
    pool_.join();
}

bool ServerController::ValidateArguments(int arg_count, char** arg_values) {
    if (arg_count == 2) return true;

    std::cerr << "Execution format: " << arg_values[0]
              << " <port_number>" << std::endl;
    return false;
}

void ServerController::InitializeNetworkComponents(
    io_context& ctx,
    const char* port
) {
    static boost::asio::thread_pool pool(std::thread::hardware_concurrency());

    static auto work = make_work_guard(pool.get_executor());

    auto listener = boost::make_shared<ConnectionAcceptor>(
        ctx,
        ip::tcp::endpoint(ip::tcp::v4(), std::stoi(port))
    );

    boost::asio::post(pool, [listener] {
        try {
            listener->Start();
        } catch (const std::exception& e) {
            std::cerr << "Listener error: " << e.what() << std::endl;
        }
    });
}

void ServerController::SetupSignalHandling() {
    signal(SIGINT, [](int) { g_force_exit.store(true); });
    signal(SIGTERM, [](int) { g_force_exit.store(true); });
}

ServerController::ThreadPoolWrapper::ThreadPoolWrapper(const size_t threads)
    : pool_(threads) {}

void ServerController::ThreadPoolWrapper::Submit(io_context& ctx) {
    post(pool_, [&ctx] { ctx.run(); });
}

ServerController::ThreadPoolWrapper::~ThreadPoolWrapper() {
    pool_.join();
}