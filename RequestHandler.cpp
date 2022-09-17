#include "RequestHandler.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

std::vector<std::string> RequestHandler::split_http_header(const std::string &s,
                                                           char delimiter) {
    auto idx = s.find(delimiter);
    std::vector<std::string> tokens;
    if (idx != std::string::npos) {
        std::string key = s.substr(0, idx);
        std::string value = s.substr(idx + 1, s.size());
        boost::trim(key);
        boost::trim(value);
        tokens.push_back(std::move(key));
        tokens.push_back(std::move(value));
    }

    return tokens;
}

RequestHandler::~RequestHandler() { socket_.close(); }

bool RequestHandler::accept_header(std::string &header) {
    for (auto &accepted_header : accepted_headers) {
        if (boost::algorithm::starts_with(header, accepted_header)) {
            return true;
        }
    }
    return false;
}

void RequestHandler::print_connection_info() {
    std::cout << "Platform shr_ptr count: " << this->platform.use_count()
              << std::endl;
    auto local_endpoint = this->socket_.local_endpoint();
    auto remote_endpoint = this->socket_.remote_endpoint();
    std::cout << "New connection from " << remote_endpoint.address().to_string()
              << ":" << remote_endpoint.port() << " to "
              << local_endpoint.address().to_string() << ":"
              << local_endpoint.port() << std::endl;
}

bool RequestHandler::process_request_metadata() {
    std::string message = read_(socket_);
    boost::split_regex(tokens, message, boost::regex("(\r\n)+"));

    if (tokens.size() < 2) {
        return false;  // HTTP1.0 //request //GET \r\n\r\n
    }

    std::vector<std::string> split_container;
    boost::split(split_container, tokens.front(), boost::is_any_of(" "));

    if (split_container.size() != 3) {
        return false;
    }

    request_data.method = split_container[0];
    request_data.path = split_container[1];
    request_data.version = split_container[2];
    request_data.body = tokens.back();

    std::printf("Method: %s -- Path: %s -- Version: %s\n",
                request_data.method.c_str(), request_data.path.c_str(),
                request_data.version.c_str());

    tokens.pop_back();
    tokens.pop_front();

    return true;
}

void RequestHandler::bad_request() {
    send_(socket_, "HTTP/1.0 400 Bad Request\r\n\r\n");
}

RequestHandler::RequestHandler(tcp::socket socket,
                               const std::shared_ptr<Platform> platform)
    : socket_(std::move(socket)), public_path{"public"}, platform{platform} {
    print_connection_info();
    accepted_headers.emplace_back("Content-Type");
    accepted_headers.emplace_back("Accept");
    accepted_headers.emplace_back("Host");
}

void RequestHandler::handle() {
    if (!process_request_metadata()) {
        bad_request();
        return;
    }

    for (const auto &token : tokens) {
        auto header_data = split_http_header(token);
        if (header_data.size() == 2 && accept_header(header_data.front())) {
            request_data.headers[header_data.front()] = header_data.back();
        }
    }

    if (request_data.method == "GET" || request_data.method == "HEAD") {
        handle_get_request();
    } else if (request_data.method == "POST") {
        boost::json::value response_value = handle_post_request();
        std::string serialized_json = boost::json::serialize(response_value);
        std::string request_response =
            "HTTP/1.1 200 OK\r\n\r\n" + serialized_json;
        send_(socket_, request_response);
    } else {
        bad_request();
    }
}

std::string RequestHandler::read_(tcp::socket &socket) {
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");
    std::string data = boost::asio::buffer_cast<const char *>(buf.data());
    return data;
}

void RequestHandler::send_(tcp::socket &socket, const std::string &message) {
    const std::string msg = message + "\n";
    boost::asio::write(socket, boost::asio::buffer(message));
}

void RequestHandler::handle_get_request() {
    if(request_data.method == "HEAD") {
        std::string response;
        response.append("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        send_(socket_, response);
        return;
    }

    auto bin_path = platform->get_executable_path();

    std::string utf8_bin_path = platform->get_platform_string(bin_path);
    boost::trim(utf8_bin_path);
    std::string file_path;
    file_path.append(utf8_bin_path.data())
        .append("/")
        .append(public_path)
        .append(request_data.path);
    std::ifstream file(file_path, std::ios::binary);

    if (file.good() && file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        std::string response;
        response.append("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n")
            .append(content);
        send_(socket_, response);
    } else {
        std::cout << "File Not Found" << std::endl;
        send_(socket_, "HTTP/1.1 404 Not Found\r\n\r\n");
    }
}

boost::json::value RequestHandler::handle_post_request() {
    // Ué, to aqui ainda
    boost::json::parse_options opts;
    opts.allow_comments = true;
    opts.allow_invalid_utf8 = false;
    opts.allow_trailing_commas = true;
    auto doc = boost::json::parse(request_data.body);
    // std::cout << doc.at("ola") << std::endl;

    for (auto &value : doc.as_object()) {
        value.value() = value.value().as_string().c_str() +
                        std::string(" - Dado processado.");

        std::cout << value.value() << std::endl;
    }
    std::cout << doc << std::endl;
    return doc;
}