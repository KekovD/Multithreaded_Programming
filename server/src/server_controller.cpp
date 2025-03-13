#include "../include/server_controller.h"
#include "../include/connection_acceptor.h"
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

boost::atomic<bool> ServerController::g_force_exit{false};

int ServerController::RunService(int argc, char** argv) {
    if (!ValidateArguments(argc, argv)) return 1;

    try {
        std::cout << "Starting server..." << std::endl;
        io_context io_ctx;
        auto work = boost::asio::make_work_guard(io_ctx);

        SetupSignalHandling();
        InitializeNetworkComponents(io_ctx, argv[1]);

        std::cout << "Server started on port " << argv[1] << std::endl;
        std::cout.flush();

        std::vector<std::thread> threads;
        constexpr size_t num_threads = 4;
        std::cout << "Creating " << num_threads << " io_context threads" << std::endl;

        for(size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&io_ctx, i] {
                std::cout << "Thread " << i << " started" << std::endl;
                try {
                    io_ctx.run();
                    std::cout << "Thread " << i << " finished normally" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Thread " << i << " exception: " << e.what() << std::endl;
                }
            });
        }

        std::cout << "Main thread waiting for exit signal" << std::endl;
        while(!g_force_exit.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Stopping io_context..." << std::endl;
        io_ctx.stop();

        std::cout << "Joining threads..." << std::endl;
        for(auto& t : threads) {
            if(t.joinable()) {
                t.join();
            }
        }

        std::cout << "Server stopped" << std::endl;

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
    try {
        std::cout << "Creating connection acceptor on port " << port << std::endl;

        static auto listener = boost::make_shared<ConnectionAcceptor>(
            ctx,
            boost::asio::ip::tcp::endpoint(
                boost::asio::ip::tcp::v4(),
                std::stoi(port)
            ),
            4
        );

        std::cout << "Starting connection acceptor..." << std::endl;
        listener->Start();
        std::cout << "Connection acceptor started successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing network components: " << e.what() << std::endl;
        throw;
    }
}

void ServerController::SetupSignalHandling() {
    std::cout << "Setting up signal handlers..." << std::endl;
    signal(SIGINT, [](int) {
        std::cout << "Received SIGINT, shutting down..." << std::endl;
        g_force_exit.store(true);
    });

    signal(SIGTERM, [](int) {
        std::cout << "Received SIGTERM, shutting down..." << std::endl;
        g_force_exit.store(true);
    });
    std::cout << "Signal handlers set up" << std::endl;
}

ServerController::ThreadPoolWrapper::ThreadPoolWrapper(const size_t threads)
    : pool_(threads) {}

void ServerController::ThreadPoolWrapper::Submit(io_context& ctx) {
    boost::asio::post(pool_, [&ctx] { ctx.run(); });
}

ServerController::ThreadPoolWrapper::~ThreadPoolWrapper() {
    pool_.join();
}