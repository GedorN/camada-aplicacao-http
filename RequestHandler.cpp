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

RequestHandler::~RequestHandler() {
    socket_.close();
    std::cout << "Socket Destroyed" << std::endl;
}

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

void RequestHandler::not_found() {
    std::string file_path{get_request_file_path("404.html")};
    std::ifstream file(file_path, std::ios::binary);

    auto response =
        HttpResponse::create().set_status(HttpStatusCode::NOT_FOUND);
    if (file.good() && file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        response.set_body(content)
            .set_header("Content-Type", get_file_mime_type(file_path))
            .set_header("Content-Length", std::to_string(content.size()));
    } else {
        response.set_body("404 Not Found").set_header("Content-Length", "13");
    }

    // todo!  NOT FOUND. ESSA CHAMADA FUNCIONA, porém olhar TODO!(2)
    send_(socket_, response.to_string());
}

RequestHandler::RequestHandler(tcp::socket socket,
                               const std::shared_ptr<Platform> platform)
    : socket_(std::move(socket)), public_path{"public"}, platform{platform} {
    print_connection_info();
    accepted_headers.emplace_back("Content-Type");
    accepted_headers.emplace_back("Accept");
    accepted_headers.emplace_back("Host");
}

void RequestHandler::internal_error() {
    std::string str{"500 Internal Server Error"};
    auto response =
        HttpResponse::create()
            .set_status(HttpStatusCode::INTERNAL_SERVER_ERROR)
            .set_body(str)
            .set_header("Content-Length", std::to_string(str.size()));
    send_(socket_, response.to_string());
}

void RequestHandler::handle() {
    try {
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

        if (request_data.method == "GET") {
            handle_get_request();
        } else if (request_data.method == "POST") {
            auto content = handle_post_request();
            auto response = HttpResponse::create();
            response.set_status(HttpStatusCode::OK)
                .set_header("Content-Type", "application/json")
                .set_header("Content-Length", std::to_string(content.size()))
                .set_body(content);

            send_(socket_, response.to_string());
        } else if (request_data.method == "HEAD") {
            handle_head_request();
        } else {
            bad_request();
        }
    } catch (const std::exception &e) {
        internal_error();
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
    boost::asio::write(socket, boost::asio::buffer(msg));
}

std::string RequestHandler::get_file_mime_type(std::string &file_path) {
    std::string file_extension =
        file_path.substr(file_path.find_last_of(".") + 1, file_path.size());
    if (file_extension == "html") {
        return "text/html";
    } else if (file_extension == "css") {
        return "text/css";
    } else if (file_extension == "js") {
        return "application/javascript";
    } else if (file_extension == "png") {
        return "image/png";
    } else if (file_extension == "jpg") {
        return "image/jpg";
    } else if (file_extension == "jpeg") {
        return "image/jpeg";
    } else if (file_extension == "gif") {
        return "image/gif";
    } else if (file_extension == "svg") {
        return "image/svg+xml";
    } else if (file_extension == "ico") {
        return "image/x-icon";
    } else if (file_extension == "json") {
        return "application/json";
    } else if (file_extension == "xml") {
        return "application/xml";
    } else if (file_extension == "pdf") {
        return "application/pdf";
    } else if (file_extension == "zip") {
        return "application/zip";
    } else if (file_extension == "rar") {
        return "application/x-rar-compressed";
    } else if (file_extension == "gz") {
        return "application/gzip";
    } else if (file_extension == "txt") {
        return "text/plain";
    } else if (file_extension == "mp3") {
        return "audio/mpeg";
    } else if (file_extension == "wav") {
        return "audio/x-wav";
    } else if (file_extension == "mpeg") {
        return "video/mpeg";
    } else if (file_extension == "mpg") {
        return "video/mpeg";
    } else if (file_extension == "mpe") {
        return "video/mpeg";
    } else if (file_extension == "mov") {
        return "video/quicktime";
    } else if (file_extension == "avi") {
        return "video/x-msvideo";
    } else if (file_extension == "3gp") {
        return "video/3gpp";
    }

    return "";
}

std::string RequestHandler::get_request_file_path(std::string file_name) {
    if (file_name.empty()) {
        file_name = request_data.path;
    } else if (!file_name.starts_with("/")) {
        file_name = "/" + file_name;
    }

    std::string file_path{platform->get_bin_path()};
    file_path.append("/").append(public_path).append(file_name);

    if (file_path.back() == '/') {
        file_path.append("index.html");
    }

    return file_path;
}

void RequestHandler::handle_head_request() {
    std::string file_path{get_request_file_path()};

    auto response =
        HttpResponse::create()
            .set_status(HttpStatusCode::OK)
            .set_header("Content-Type", get_file_mime_type(file_path));
    send_(socket_, response.to_string());
}

std::string RequestHandler::get_request_mime_type() {
    return this->request_data.headers["Content-Type"];
}

void RequestHandler::handle_get_request() {
    /* TODO!(2) - Na chamada no NOT FOUND, existe uma imagem no HTML, que volta
    como um GET pra imagem da pasta public.*/
    /*TODO!(2) - Essa imagem é retornada corretamente, porém, após o envio,
    gera uma nova conexão que não é fechada. (Trava na função _read)*/
    std::string file_path{get_request_file_path()};
    std::ifstream file(file_path, std::ios::binary);

    if (file.good() && file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        auto request =
            HttpResponse::create()
                .set_status(HttpStatusCode::OK)
                .set_header("Content-Type", get_file_mime_type(file_path))
                .set_header("Content-Length", std::to_string(content.size()))
                .set_body(content);

        send_(socket_, request.to_string());
    } else {
        file.close();
        not_found();
    }
}

std::string RequestHandler::handle_post_request() {
    auto req_mime_type = get_request_mime_type();

    if (req_mime_type == "application/json") {
        if (platform->get_platform_type() == PlatformType::Windows) {
            // TODO! NÃO FUNFA NO WINDOWS AINDA
            auto processed_data =
                request_data.body.append(" - Dado processado.");

            return processed_data;
            if (!request_data.body.starts_with("\"")) {
                request_data.body = "\"" + request_data.body;
            }

            if (!request_data.body.ends_with("\"")) {
                request_data.body = request_data.body + "\"";
            }
        }

        boost::json::parse_options opts;
        opts.allow_comments = true;
        opts.allow_invalid_utf8 = false;
        opts.allow_trailing_commas = true;
        auto doc = boost::json::parse(request_data.body);

        for (auto &value : doc.as_object()) {
            value.value() = value.value().as_string().c_str() +
                            std::string(" - Dado processado.");

            std::cout << value.value() << std::endl;
        }
        std::cout << doc << std::endl;
        std::string serialized_json = boost::json::serialize(doc);
        return serialized_json;
    } else {
        auto processed_data = request_data.body.append(" - Dado processado.");

        return processed_data;
    }
}