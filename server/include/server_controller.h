#ifndef SERVER_CONTROLLER_H
#define SERVER_CONTROLLER_H

#include <boost/asio.hpp>
#include <boost/atomic.hpp>

using boost::system::error_code;

class ServerController {
public:
    static int RunService(int argc, char** argv);

    static boost::atomic<bool> g_force_exit;

private:
    static bool ValidateArguments(int arg_count, char** arg_values);
    static void InitializeNetworkComponents(
        boost::asio::io_context& ctx,
        const char* port
    );
    static void SetupSignalHandling();

    class ThreadPoolWrapper {
    public:
        void WaitForCompletion();
        explicit ThreadPoolWrapper(size_t threads);
        void Submit(boost::asio::io_context& ctx);
        ~ThreadPoolWrapper();
        
    private:
        boost::asio::thread_pool pool_;
    };
};

#endif // SERVER_CONTROLLER_H