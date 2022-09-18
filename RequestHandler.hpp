#include <map>
#include <deque>
#include "Platform.hpp"
#include <boost/json.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace net = boost::asio;  // from <boost/asio.hpp>
using tcp = net::ip::tcp;     // from <boost/asio/ip/tcp.hpp>

struct HttpRequestData {
    std::string body;
    std::string method;
    std::string path;
    std::string version;
    std::string query;
    std::map<std::string, std::string> query_params;
    std::map<std::string, std::string> headers;
};

class RequestHandler {
   private:
    const std::shared_ptr<Platform> platform;
    HttpRequestData request_data;
    tcp::socket socket_;
    std::deque<std::string> tokens;
    std::vector<std::string> accepted_headers;
    std::string public_path;

   public:
    RequestHandler(tcp::socket socket,
                   const std::shared_ptr<Platform> platform);
    std::vector<std::string> split_http_header(const std::string &s,
                                               char delimiter = ':');
    ~RequestHandler();
    std::string handle_post_request();
    void handle_get_request();
    std::string read_(tcp::socket &socket);
    void send_(tcp::socket &socket, const std::string &message);
    void handle();
    void bad_request();
    bool process_request_metadata();
    bool accept_header(std::string &header);
    void print_connection_info();
};
