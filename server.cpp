#include "RequestHandler.hpp"
#include <future>
#include <iostream>

int main() {
    const std::shared_ptr<Platform> platform = Platform::create_platform();
    platform->setup();

    boost::asio::io_service io_service;
    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 3005));

    while (true) {
        std::cout << "Platform shr_ptr count: " << platform.use_count()
                  << std::endl;
        tcp::socket socket_(io_service);
        acceptor_.accept(socket_);

        // Thread normal, pois o async usa threadpool. Precisamos de um novo
        // contexto.
        std::thread(
            [](tcp::socket socket_, const std::shared_ptr<Platform> platform) {
                try {
                    RequestHandler handler(std::move(socket_), platform);
                    handler.handle();
                } catch (const std::exception& e) {
                    std::cerr << e.what() << '\n';
                }
            },
            std::move(socket_), platform)
            .detach();

        std::cout << "Platform shr_ptr count: " << platform.use_count()
                  << std::endl;
    }

    return 0;
}