#include "server.hpp"
#include "parser.hpp"
#include <winsock2.h>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

// Read one line from a socket, stripping \r\n.
// Returns nullopt on EOF or socket error — callers must treat this as "close connection".
static std::optional<std::string> readLine(SOCKET sock) {
    std::string line;
    char c;
    while (true) {
        int n = recv(sock, &c, 1, 0);
        // n == 0 is orderly shutdown; n < 0 is an error — both mean stop reading.
        if (n <= 0) return std::nullopt;
        if (c == '\n') return line;
        if (c != '\r') line += c;
    }
}

static void sendAll(SOCKET sock, const std::string& msg) {
    const char* ptr = msg.c_str();
    int remaining = static_cast<int>(msg.size());
    while (remaining > 0) {
        int sent = send(sock, ptr, remaining, 0);
        if (sent == SOCKET_ERROR) return;
        ptr += sent;
        remaining -= sent;
    }
}

static void handleClient(SOCKET clientSocket, KVStore& store) {
    while (true) {
        auto lineOpt = readLine(clientSocket);
        if (!lineOpt) break;

        const std::string& line = *lineOpt;
        if (line.empty()) continue;

        Command cmd = parse(line);
        std::string response;

        switch (cmd.type) {
            case CommandType::SET:
                store.set(cmd.key, cmd.value);
                response = "OK\r\n";
                break;
            case CommandType::GET: {
                auto val = store.get(cmd.key);
                response = val ? *val + "\r\n" : "NOT_FOUND\r\n";
                break;
            }
            case CommandType::DEL:
                response = store.del(cmd.key) ? "OK\r\n" : "NOT_FOUND\r\n";
                break;
            case CommandType::EXIT:
                sendAll(clientSocket, "BYE\r\n");
                closesocket(clientSocket);
                return;
            case CommandType::UNKNOWN:
                response = "ERROR " + cmd.error + "\r\n";
                break;
        }

        sendAll(clientSocket, response);
    }

    closesocket(clientSocket);
}

Server::Server(int port) : port_(port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

Server::~Server() {
    WSACleanup();
}

void Server::run() {
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        throw std::runtime_error("socket() failed: " + std::to_string(WSAGetLastError()));
    }

    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<u_short>(port_));

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        throw std::runtime_error("bind() failed: " + std::to_string(WSAGetLastError()));
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        throw std::runtime_error("listen() failed: " + std::to_string(WSAGetLastError()));
    }

    std::cout << "KV server listening on port " << port_ << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;

        // Detach is intentional: run() never returns, so the Server outlives every client thread.
        // store_ is thread-safe via its internal mutex, so sharing it across threads is safe.
        std::thread([clientSocket, this]() {
            handleClient(clientSocket, store_);
            std::cout << "Client disconnected" << std::endl;
        }).detach();
    }

    closesocket(listenSocket); // unreachable today; left for when a shutdown path is added
}
