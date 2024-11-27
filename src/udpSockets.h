#pragma once

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <memory>

#include "util.h"

using namespace std;

class SocketUDP : private NonCopyableNonMovable {
    int socket;

   public:
    SocketUDP();
    ~SocketUDP();
    int getSocket() const;

    // TODO: enable move semantics

   private:
    bool canCloseSocket() { return true; }  // TODO: implement this
};

// services that the load balancer is distributing traffic across
class ServiceSocketUDP : private NonCopyableNonMovable {
    SocketUDP socket;
    struct sockaddr_in destAddr;

   public:
    ServiceSocketUDP(const char* ip, uint16_t port);
    ~ServiceSocketUDP();
    int send(const char* message);
};

// socket that server reads incoming udp traffic from services
// can be bound
class ServerSocketUDP : private NonCopyableNonMovable {
    SocketUDP socket;

   public:
    void bind(const sockaddr* addr, const socklen_t len);  // attempts to bind to port based on settings in sockaddr
};

struct ClientIdentifier {
    uint32_t ip;    // IP address in binary form
    uint16_t port;  // Port in binary form

    bool operator==(const ClientIdentifier& other) const {
        return ip == other.ip && port == other.port;
    };
};

template <>
struct hash<ClientIdentifier> {
    size_t operator()(const ClientIdentifier& client) const {
        return std::hash<uint32_t>()(client.ip) ^ std::hash<uint16_t>()(client.port);
    }
};

// incoming packet read from server socket
struct Packet {
    ClientIdentifier sender;

    // TODO: what goes here?
};
