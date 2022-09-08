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
#include <vector>
#include <future>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <boost/json.hpp>
#include <boost/json/src.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

static int request_counter = 0;

std::vector<std::string> split(const std::string& s, char delimiter) {
  int index = 0;
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }

  if(tokens.size() == 3) {
    tokens[1] = tokens[1] + ":" + tokens[2];
    tokens.pop_back();
  }

  return tokens;
}

class RequestHandler
{
private:
    tcp::socket socket_;
    std::vector<std::string> tokens;
    std::map<std::string, std::string> headers;
    std::vector<std::string> accepted_headers;
    std::string public_path;
    std::string body;

public:
    // Recebe o socket(que é um objeto que foi movido), e move novamente.
    // Dessa vez move para ser um objeto persistente desse objeto(this)
    RequestHandler(tcp::socket socket) : socket_(std::move(socket)),
    public_path{"./public"}
    {
        accepted_headers.emplace_back("Content-Type");
        accepted_headers.emplace_back("Accept");
        accepted_headers.emplace_back("Host");
    }

    void handle()
    {
        std::string message = read_(socket_);
        std::cout << message << std::endl;
        boost::split_regex(tokens, message, boost::regex("(\r\n)+"));
        if (tokens.empty())
        {
            send_(socket_, "HTTP/1.0 400 Bad Request\r\n\r\nSE FUDEU: " +
                               std::to_string(request_counter));
            socket_.close();
            return;
        }

        std::vector<std::string> first_line;
        boost::split(first_line, tokens[0], boost::is_any_of(" "));

        std::string method = first_line[0];
        std::cout << "veja o método: " << method << std::endl;
        std::cout << "Tokens Size: " << tokens.size() << std::endl;
        body = tokens[tokens.size() - 1];
        

        for(int i = 1;  i < tokens.size(); i++)
        {
            std::vector<std::string> header_tokens;
            header_tokens = split(tokens[i], ':');
            //boost::split_regex(header_tokens, tokens[i], boost::regex("^.+?(?=\:)"));
            std::cout << "demonio: " << tokens[i] << std::endl;
            boost::trim(header_tokens[0]);
            boost::trim(header_tokens[1]);
            std::cout << "TO vendo o resultado: " << header_tokens[1] << std::endl;

            if(header_tokens.size() != 2) continue;
            std::cout << "TO vendo o token: " << header_tokens[0] << std::endl;

            std::string header = header_tokens[0];
            if(std::find(accepted_headers.begin(), accepted_headers.end(), header) != accepted_headers.end())
            {
                std::cout << "Aceitei essa safada " << header_tokens[0] << std::endl;
                std::string value = header_tokens[1];
                headers[header] = value;
                std::cout << "Atribui nessa safada " << header_tokens[1] << std::endl;
            }
        }
        
        std::cout<< "Body: " << body << std::endl;

        if (method == "GET"){
            handle_get_request();

            send_(socket_, "HTTP/1.1 200 OK\r\n\r\nTESTE: " +
                           std::to_string(request_counter));
        }
        else if (method == "POST"){
            boost::json::value response_value = handle_post_request();
            std::string serialized_json = boost::json::serialize( response_value ); 
            std::string request_response = "HTTP/1.1 200 OK\r\n\r\n" + serialized_json;
            send_(socket_, request_response);

        }
        request_counter++;
        socket_.close();
    }

    std::string read_(tcp::socket &socket)
    {
        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, "\n");
        std::string data = boost::asio::buffer_cast<const char *>(buf.data());
        return data;
    }

    void send_(tcp::socket &socket, const std::string &message)
    {
        const std::string msg = message + "\n";
        boost::asio::write(socket, boost::asio::buffer(message));
    }

    void handle_get_request()
    {
        // std::string path = tokens[index++];
        // std::cout << "GET " << path << std::endl;
        
    }

    boost::json::value handle_post_request() {
        // Ué, to aqui ainda
        boost::json::parse_options opts;
        opts.allow_comments = true;
        opts.allow_invalid_utf8 = false;
        opts.allow_trailing_commas = true;
        auto doc = boost::json::parse(body);
        // std::cout << doc.at("ola") << std::endl;

        for (auto &value : doc.as_object())
        {
            value.value() = value.value().as_string().c_str() + std::string(" - Dado processado.");
            // auto json_v = value.value().as_string();
            // std::cout << json_v << std::endl;
            // std::cout << typeid(value.value().get_allocator()).name() << std::endl;
            // std::cout << typeid(value.value().storage().get()).name() << std::endl;
            // std::cout << value.value().kind() << std::endl;
            // std::string z{"s"};
            // std::any a = z;
            // auto x = decltype(a)::type;
            // auto s = static_cast<decltype(a)>(a);

            std::cout << value.value() << std::endl;
            // value.get(0) = json_v[0] + "xablau";
            // std::cout << value.key() << " " << json_v[0] << std::endl;
        }
        std::cout << doc << std::endl;
        return doc;

        
    }
};

int main()
{
    // std::vector<std::thread> request_threads;

    boost::asio::io_service io_service;
    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 3005));

    while (true)
    {
        // Inicializar um novo socket a cada iteração
        tcp::socket socket_(io_service);
        // acceptor usa o socket para receber uma request
        acceptor_.accept(socket_);
        // cria um thread para lidar com a requisição
        /*thread é passado como um movable pq ao final do escopo vai ser
         *deletado, então precisa manter o lifetime dele*/
        // Da um detach pra destruir dps que a função termina
        std::async(
            std::launch::async,
            [](tcp::socket socket_)
            {
                RequestHandler handler(std::move(socket_));
                handler.handle();
            },
            std::move(socket_));

        // .detach();
    }

    return 0;
}