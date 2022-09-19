#include <map>
#include <deque>
#include "Platform.hpp"
#include <boost/json.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <map>

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

enum class HttpStatusCode {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500,
};

struct HttpResponse {
   private:
    std::map<HttpStatusCode, std::string> status_codes = {
        {HttpStatusCode::OK, "200 OK"},
        {HttpStatusCode::BAD_REQUEST, "400 Bad Request"},
        {HttpStatusCode::NOT_FOUND, "404 Not Found"},
        {HttpStatusCode::INTERNAL_SERVER_ERROR, "500 Internal Server Error"},
    };
    std::string http_version;
    std::string body;
    std::string status;
    std::map<std::string, std::string> headers;
    HttpResponse() : http_version{"HTTP/1.0"} {
        set_header("Connection", "close");
    }

   public:
    static HttpResponse create() { return HttpResponse(); }
    HttpResponse &set_status(HttpStatusCode status_code) {
        status = status_codes[status_code];
        return *this;
    }
    HttpResponse &set_body(const std::string &body) {
        this->body = body;
        return *this;
    }
    HttpResponse &set_header(const std::string &key, const std::string &value) {
        this->headers[key] = value;
        return *this;
    }
    HttpResponse &set_http_version(const std::string &version) {
        this->http_version = version;
        return *this;
    }
    std::string to_string() {
        std::string response;
        response.append(this->http_version);
        response.append(" ");
        response.append(this->status);
        response.append("\r\n");
        for (auto &header : this->headers) {
            if (header.first == "Content-Length") {
                // Sempre enviamos um \n no final, então precisa aumentar o
                // length.
                header.second =
                    std::to_string(std::atoi(header.second.c_str()) + 1);
            }

            response.append(header.first);
            response.append(": ");
            response.append(header.second);
            response.append("\r\n");
        }
        response.append("\r\n");
        response.append(this->body);

        // std::cout << response << std::endl;

        return response;
    }
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
    void handle_head_request();
    std::string read_(tcp::socket &socket);
    void send_(tcp::socket &socket, const std::string &message);
    void handle();
    void bad_request();
    void not_found();
    void internal_error();
    bool process_request_metadata();
    bool accept_header(std::string &header);
    void print_connection_info();
    std::string get_file_mime_type(std::string &file_path);
    std::string get_request_file_path(std::string file_name = "");
    std::string get_request_mime_type();
};
