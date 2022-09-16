#include "RequestHandler.hpp"
#include <future>
#include <iostream>

int main() {
    const std::shared_ptr<Platform> platform = Platform::create_platform();

    boost::asio::io_service io_service;
    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 3005));

    while (true) {
        std::cout << "Platform shr_ptr count: " << platform.use_count()
                  << std::endl;
        // Inicializar um novo socket a cada iteração
        tcp::socket socket_(io_service);
        // acceptor usa o socket para receber uma request
        acceptor_.accept(socket_);
        // cria um thread para lidar com a requisição
        /*thread é passado como um movable pq ao final do escopo vai ser
         *deletado, então precisa manter o lifetime dele*/
        // Da um detach pra destruir dps que a função termina
        auto future_result = std::async(
            std::launch::async,
            [](tcp::socket socket_, const std::shared_ptr<Platform> platform) {
                try {
                    RequestHandler handler(std::move(socket_), platform);
                    handler.handle();
                } catch (const std::exception& e) {
                    std::cerr << e.what() << '\n';
                }
            },
            std::move(socket_), platform);

        std::cout << "Platform shr_ptr count: " << platform.use_count()
                  << std::endl;
    }

    return 0;
}