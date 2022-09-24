#include "RequestHandler.hpp"
#include <future>
#include <iostream>

int main() {
    const std::shared_ptr<Platform> platform = Platform::create_platform();
    platform->setup();

    boost::asio::io_service io_service;
    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 8223));

    while (true) {
        tcp::socket socket_(io_service);
        acceptor_.accept(socket_);
        // Thread normal, pois o async usa threadpool. Precisamos de um novo
        // contexto.
        std::thread([socket_ = std::move(socket_), platform]() mutable {
            try {
                RequestHandler handler(std::move(socket_), platform);
                handler.handle();
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n';
            }
        }).detach();
    }

    return 0;
}