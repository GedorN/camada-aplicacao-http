#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <sstream>
#include <string>

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

static int request_counter = 0;

class RequestHandler {
   private:
    tcp::socket socket_;

   public:
    // Recebe o socket(que é um objeto que foi movido), e move novamente.
    // Dessa vez move para ser um objeto persistente desse objeto(this)
    RequestHandler(tcp::socket socket) : socket_(std::move(socket)) {}

    void handle() {
        std::string message = read_(socket_);
        std::cout << message << std::endl;

        send_(socket_, "HTTP/1.1 200 OK\r\n\r\nTESTE: " +
                           std::to_string(request_counter));
        request_counter++;
        socket_.close();
    }

    std::string read_(tcp::socket& socket) {
        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, "\n");
        std::string data = boost::asio::buffer_cast<const char*>(buf.data());
        return data;
    }

    void send_(tcp::socket& socket, const std::string& message) {
        const std::string msg = message + "\n";
        boost::asio::write(socket, boost::asio::buffer(message));
    }
};

int main() {
    std::vector<std::thread> request_threads;

    boost::asio::io_service io_service;
    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 3005));

    while (true) {
        // Inicializar um novo socket a cada iteração
        tcp::socket socket_(io_service);
        // acceptor usa o socket para receber uma request
        acceptor_.accept(socket_);
        // cria um thread para lidar com a requisição
        /*thread é passado como um movable pq ao final do escopo vai ser
         *deletado, então precisa manter o lifetime dele*/
        // Da um detach pra destruir dps que a função termina
        request_threads
            .emplace_back(
                [](tcp::socket socket_) {
                    RequestHandler handler(std::move(socket_));
                    handler.handle();
                },
                std::move(socket_))
            .detach();
    }

    return 0;
}