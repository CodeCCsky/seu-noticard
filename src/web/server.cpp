/*
临时服务，后续换成 WebServer
*/
// 让 windows.h 不自动把 winsock.h 导进来
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>  // winsock2.h 必须放在 windows.h 前面，防止 windows.h 把 winsock.h 带进来污染代码
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
// 此处不pragma comment 连接 ws2_32.lib 和 shell32.lib
// CMake 统一管，防止改了一处忘记另一处要改

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

#include "util/status.h"
#include "web/server.h"

namespace nc {
namespace web {

namespace {

constexpr std::size_t kMaxRequestBytes = 8 * 1024;  // 请求头上限 8KB

class WsaGuard {
    // 保证 winsock 在任何时候都能自动 WSACleanup
public:
    WsaGuard() = default;
    ~WsaGuard() {
        if (started_) ::WSACleanup();
    }  // 从全局开始找 WSACleanup，防止被遮蔽，下面同理
    WsaGuard(const WsaGuard&) = delete;             // 不允许拷贝构造，防止双重释放等问题
    WsaGuard& operator=(const WsaGuard&) = delete;  // 不允许拷贝赋值构造，同上
    WsaGuard(WsaGuard&&) = delete;                  // 不允许移动构造，因为没必要，它只是个守卫而已
    WsaGuard& operator=(WsaGuard&&) = delete;       // 不允许移动赋值构造，同上

    void mark_started() { started_ = true; }

private:
    bool started_ = false;
};

class SocketGuard {
public:
    explicit SocketGuard(SOCKET s = INVALID_SOCKET) : s_(s) {}  // 禁止隐式转换
    ~SocketGuard() { reset(); }                                 // 析构时自动关闭 socket
    SocketGuard(const SocketGuard&) = delete;                   // 不允许拷贝，一个 socket 一个 guard
    SocketGuard& operator=(const SocketGuard&) = delete;        // 同上

    // TODO 移动构造确实可以写，现在暂时不用，后续扩展时视情况添加

    SOCKET get() const { return s_; }
    void reset(SOCKET s = INVALID_SOCKET) {
        if (s_ != INVALID_SOCKET) {
            ::closesocket(s_);
        }
        s_ = s;
    }

private:
    SOCKET s_;
};

struct Response {
    int status = 200;
    std::string reason = "OK";
    std::string content_type;  // 为空时即没有这个头
    std::string body;
    bool has_body = true;  // 204/304 时为 false，此时无 content_type 和 body
};

std::string serialize(const Response& res) {
    // 序列化
    std::ostringstream oss;
    oss << "HTTP/1.1 " << res.status << ' ' << res.reason << "\r\nConnection: close\r\n";

    if (res.has_body) {
        if (!res.content_type.empty()) {
            oss << "Content-Type: " << res.content_type << "\r\n";
        }
        oss << "Content-Length: " << res.body.size() << "\r\n";  // 字节数
    }
    oss << "\r\n";
    if (res.has_body) {
        oss << res.body;
    }
    return oss.str();
}

bool send_all(SOCKET s, const std::string& data) {
    // send 一次可能只能发出去一部分，需要循环
    // 不用 Status 因为处理这里异常的方法只有放弃链接一种
    std::size_t sent = 0;
    while (sent < data.size()) {
        const int n = ::send(s, data.data() + sent, static_cast<int>(data.size() - sent), 0);
        if (n == SOCKET_ERROR || n == 0) {
            return false;
        }
        sent += static_cast<std::size_t>(n);
    }
    return true;
}

void send_response(SOCKET client, const std::string& response) {
    if (!send_all(client, response)) {
        std::cerr << "send failed: " << ::WSAGetLastError() << "\n";
    }
}

enum class ReadResult { Ok, Closed, Timeout, TooLarge, Error};

bool read_request(SOCKET s, std::string& out) {
    // 不用 Status 理由同上
    // 但是如果还要加东西，例如想把这几种错误区分开也可以用 Status
    // 等加路由时，可以拓展到 enum 枚举错误
    out.clear();
    out.reserve(kMaxRequestBytes);
    char buf[1024];
    while (true) {
        const int n = ::recv(s, buf, static_cast<int>(sizeof(buf)), 0);
        if (n <= 0) {
            return false;  // 出错或者对方关闭了连接
        }
        out.append(buf, static_cast<std::size_t>(n));

        if (out.size() > kMaxRequestBytes) {
            return false;
        }
        if (out.find("\r\n\r\n") != std::string::npos) {
            return true;
        }
    }
}

void handle_client(SOCKET client) {
    DWORD timeout_ms = 5000;  // 超时 5s
    if (::setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout_ms),
                     static_cast<int>(sizeof(timeout_ms))) == SOCKET_ERROR) {
        std::cerr << "setsockopt SO_RCVTIMEO failed: " << ::WSAGetLastError() << "\n";
    }

    std::string raw_content;
    if (!read_request(client, raw_content)) {
        return;
    }

    const std::size_t line_end = raw_content.find("\r\n");
    const std::string line = (line_end == std::string::npos) ? raw_content : raw_content.substr(0, line_end);

    std::istringstream iss(line);
    std::string method, target, version;
    Response res;
    if (!(iss >> method >> target >> version)) {
        res.status = 400;
        res.reason = "Bad Request";
        res.content_type = "text/plain; charset=utf-8";
        res.body = "bad request";
        std::cout << method << ' ' << target << ' ' << res.status << '\n';
        send_response(client, serialize(res));
        return;
    }
    // 暂时不校验version

    if (method != "GET") {
        res.status = 405;
        res.reason = "Method Not Allowed";
        res.content_type = "text/plain; charset=utf-8";
        res.body = "method not allowed";
        std::cout << method << ' ' << target << ' ' << res.status << '\n';
        send_response(client, serialize(res));
        return;
    }

    // 去 query
    const std::size_t query = target.find('?');
    if (query != std::string::npos) {
        target.resize(query);
    }

    std::string response;
    if (target == "/") {
        const std::string body =
            u8"<!doctype html><html><head><meta charset=\"utf-8\"><title>seu-noticard</title></head><body><h1>Hello, "
            u8"world!</h1></body></html>";
        res.status = 200;
        res.reason = "OK";
        res.content_type = "text/html; charset=utf-8";
        res.body = body;
    } else if (target == "/api/health") {
        res.status = 200;
        res.reason = "OK";
        res.content_type = "application/json; charset=utf-8";
        res.body = "{\"ok\":true}";
    } else if (target == "/favicon.ico") {
        res.status = 204;
        res.reason = "No Content";
        res.has_body = false;
    } else {
        res.status = 404;
        res.reason = "Not Found";
        res.content_type = "text/plain; charset=utf-8";
        res.body = "not found";
    }
    std::cout << method << ' ' << target << ' ' << res.status << '\n';
    send_response(client, serialize(res));
    return;
}

void open_browser(int port) {
    // 自动打开浏览器，后续可换为自动打开桌面窗口？
    const std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
    std::cout << "running on: " << url << "\n";
    const HINSTANCE result = ::ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        std::cerr << "open browser failed: " << reinterpret_cast<INT_PTR>(result) << "\n";
        std::cout << "自动唤起浏览器失败，请手动打开\n";
    }
}

}  // namespace

Status run_hello_server(int port) {
    if (port < 1 || port > 65535) {
        return Status::Error("port unavailable");
    }
    WsaGuard wsa;
    WSADATA wsa_data;
    const int rc = ::WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (rc != 0) {
        return Status::Error("WSAStartup failed: " + std::to_string(rc));
    }
    wsa.mark_started();

    SocketGuard listener(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listener.get() == INVALID_SOCKET) {
        return Status::Error("socket failed: " + std::to_string(::WSAGetLastError()));
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(port));
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 只走本地

    if (::bind(listener.get(), reinterpret_cast<sockaddr*>(&addr), static_cast<int>(sizeof(addr))) == SOCKET_ERROR) {
        return Status::Error("bind 127.0.0.1:" + std::to_string(port) +
                             " failed: " + std::to_string(::WSAGetLastError()));
    }

    if (::listen(listener.get(), SOMAXCONN) == SOCKET_ERROR) {
        return Status::Error("listen 127.0.0.1:" + std::to_string(port) +
                             " failed: " + std::to_string(::WSAGetLastError()));
    }

    open_browser(port);

    while (true) {
        SOCKET client = ::accept(listener.get(), nullptr, nullptr);
        if (client == INVALID_SOCKET) {
            const int err = ::WSAGetLastError();
            if (err == WSAEINTR) {
                break;
            }
            std::cerr << "accept failed: " << err << "\n";
            Sleep(100);  // 持久错误时防止高频空转
            continue;
        }
        SocketGuard client_guard(client);
        handle_client(client_guard.get());
        // client_guard 析构时 closesocket
    }

    return Status::Ok();
}

}  // namespace web
}  // namespace nc
