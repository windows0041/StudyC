#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <cstring>
#include <stdexcept>
#include <memory>

#pragma comment(lib, "ws2_32.lib")

static constexpr int BUFFER_SIZE = 1024;
static constexpr int RECV_TIMEOUT_MS = 2000;

// ============================================================
// RAII wrapper for Winsock initialization (ref-counted by OS)
// ============================================================
class WinsockInit {
public:
    WinsockInit() {
        WSADATA wsaData;
        int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (ret != 0) {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(ret));
        }
    }
    ~WinsockInit() { WSACleanup(); }
    WinsockInit(const WinsockInit&) = delete;
    WinsockInit& operator=(const WinsockInit&) = delete;
};

// ============================================================
// Common UDP endpoint base class
// ============================================================
class UDPEndpoint {
protected:
    SOCKET _sock = INVALID_SOCKET;
    sockaddr_in _targetAddr{};
    int _addrLen = sizeof(_targetAddr);
    std::string _ip;
    std::string _port;

    std::atomic<bool> _running{false};
    std::thread _recvThread;
    std::mutex _sendMutex;

    // Virtual hook for subclasses to process incoming messages
    virtual void onReceive(const char* buffer, int len,
                           const sockaddr_in& sender) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender.sin_addr, ipStr, sizeof(ipStr));
        std::cout << "\nReceived from " << ipStr
                  << ":" << ntohs(sender.sin_port)
                  << " ==> " << buffer << std::endl;
    }

    // Receiver thread ！ runs until stop()
    void recvLoop() {
        char buffer[BUFFER_SIZE];

        while (_running) {
            sockaddr_in fromAddr{};
            int fromLen = sizeof(fromAddr);

            int recvlen = recvfrom(_sock, buffer, sizeof(buffer) - 1, 0,
                                   (sockaddr*)&fromAddr, &fromLen);

            if (recvlen == SOCKET_ERROR) {
                int err = WSAGetLastError();
                // Timeout or socket closed during stop() ！ just retry/exit
                if (err == WSAETIMEDOUT) {
                    continue;
                }
                if (_running) {
                    std::cerr << "recvfrom failed with error: " << err << std::endl;
                }
                break;
            }

            buffer[recvlen] = '\0';
            onReceive(buffer, recvlen, fromAddr);
        }
    }

    // Create and configure the UDP socket
    void createSocket() {
        _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (_sock == INVALID_SOCKET) {
            throw std::runtime_error("Socket creation failed: " +
                                     std::to_string(WSAGetLastError()));
        }

        DWORD timeout = RECV_TIMEOUT_MS;
        setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    }

public:
    UDPEndpoint(std::string ip, std::string port)
        : _ip(std::move(ip)), _port(std::move(port)) {}

    virtual ~UDPEndpoint() {
        stop();
        if (_recvThread.joinable()) {
            _recvThread.join();
        }
        if (_sock != INVALID_SOCKET) {
            closesocket(_sock);
            _sock = INVALID_SOCKET;
        }
    }

    UDPEndpoint(const UDPEndpoint&) = delete;
    UDPEndpoint& operator=(const UDPEndpoint&) = delete;

    void start() {
        _running = true;
        _recvThread = std::thread(&UDPEndpoint::recvLoop, this);
    }

    void stop() {
        _running = false;
        // closesocket() will unblock recvfrom() immediately
        if (_sock != INVALID_SOCKET) {
            closesocket(_sock);
            _sock = INVALID_SOCKET;
        }
    }

    bool sendTo(const std::string& msg) {
        std::lock_guard<std::mutex> lock(_sendMutex);
        if (_sock == INVALID_SOCKET) return false;
        int sent = sendto(_sock, msg.c_str(), static_cast<int>(msg.size()), 0,
                          reinterpret_cast<sockaddr*>(&_targetAddr), _addrLen);
        return sent != SOCKET_ERROR;
    }
};

// ============================================================
// UDP Server ！ binds to a local address
// ============================================================
class UDPServer : public UDPEndpoint {
private:
    WinsockInit _wsa;

protected:
    // Remember the last sender so sendTo() can reply
    void onReceive(const char* buffer, int len,
                   const sockaddr_in& sender) override {
        _targetAddr = sender;
        _addrLen = sizeof(sender);
        UDPEndpoint::onReceive(buffer, len, sender);
    }

public:
    UDPServer(std::string ip, std::string port)
        : UDPEndpoint(std::move(ip), std::move(port)) {
        createSocket();

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(_ip.c_str());
        serverAddr.sin_port = htons(static_cast<u_short>(std::stoi(_port)));

        if (bind(_sock, reinterpret_cast<sockaddr*>(&serverAddr),
                 sizeof(serverAddr)) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            closesocket(_sock);
            _sock = INVALID_SOCKET;
            throw std::runtime_error("Bind failed: " + std::to_string(err));
        }

        std::cout << "UDP Server running on " << _ip << ":" << _port << std::endl;
    }
};

// ============================================================
// UDP Client ！ sends to a fixed remote address
// ============================================================
class UDPClient : public UDPEndpoint {
private:
    WinsockInit _wsa;

public:
    UDPClient(std::string ip, std::string port)
        : UDPEndpoint(std::move(ip), std::move(port)) {
        createSocket();

        // Bind to INADDR_ANY:0 so the socket is immediately usable for recvfrom
        sockaddr_in localAddr{};
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = INADDR_ANY;
        localAddr.sin_port = 0;
        if (bind(_sock, reinterpret_cast<sockaddr*>(&localAddr),
                 sizeof(localAddr)) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            closesocket(_sock);
            _sock = INVALID_SOCKET;
            throw std::runtime_error("Client bind failed: " + std::to_string(err));
        }

        _targetAddr.sin_family = AF_INET;
        _targetAddr.sin_addr.s_addr = inet_addr(_ip.c_str());
        _targetAddr.sin_port = htons(static_cast<u_short>(std::stoi(_port)));
    }
};

// ============================================================
// Interactive send loop (shared by server & client)
// ============================================================
static void interactiveSendLoop(UDPEndpoint& endpoint) {
    endpoint.start();
    std::string msg;
    while (true) {
        std::cout << "Enter message to send (or 'exit' to quit): ";
        std::getline(std::cin, msg);
        if (msg == "exit" || msg == "quit") {
            break;
        }
        endpoint.sendTo(msg);
    }
}

// ============================================================
// Entry point
// ============================================================
int main(int argc, char* argv[]) {
    std::string ip = "127.0.0.1";
    std::string port = "8888";
    bool modeServer = false;
    bool modeClient = false;

    // Parse command-line arguments (order-independent)
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            ip = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            modeServer = true;
        } else if (strcmp(argv[i], "-c") == 0) {
            modeClient = true;
        }
    }

    try {
        if (modeServer) {
            UDPServer udp(ip, port);
            interactiveSendLoop(udp);
        } else if (modeClient) {
            UDPClient udp(ip, port);
            interactiveSendLoop(udp);
        } else {
            std::cerr << "Usage: " << argv[0]
                      << " -s|-c [-h <ip>] [-p <port>]" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}