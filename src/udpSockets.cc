#include "udpSockets.h"

SocketUDP::SocketUDP() : FileDescriptor(::socket(AF_INET, SOCK_DGRAM, 0)) {}

int SocketUDP::getSocket() const { return getFd(); }

ServiceSocketUDP::ServiceSocketUDP(const char* ip, uint16_t port) : SocketUDP() {
    memset(&destAddr, 0, sizeof(destAddr));    // clear struct fields to 0
    destAddr.sin_family = AF_INET;             // IPv4
    destAddr.sin_port = htons(port);           // Port number
    destAddr.sin_addr.s_addr = inet_addr(ip);  // Localhost IP address
}

ServiceSocketUDP::~ServiceSocketUDP() {}

int ServiceSocketUDP::send(const char* message) const {
    return sendto(getSocket(), message, strlen(message), 0, (struct sockaddr*)&destAddr, sizeof(destAddr));
}

ostream& operator<<(ostream& os, const ServiceSocketUDP& socket) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &socket.destAddr.sin_addr, ipStr, sizeof(ipStr));
    uint16_t port = ntohs(socket.destAddr.sin_port);

    os << ipStr << ": " << port;
    return os;
}

void ServerSocketUDP::bind(const sockaddr* addr, const socklen_t len) {
    if (::bind(getSocket(), addr, len) >= 0) binded = true;

    if (!binded) throw runtime_error("Could not bind server socket to port");
}

PacketUDP::PacketUDP() { memset(data, 0, BUFFER_SIZE); }

ostream& operator<<(ostream& os, const ClientIdentifier& client) {
    // Convert IP from binary to dotted-decimal notation
    struct in_addr addr;
    addr.s_addr = client.ip;
    char* ipStr = inet_ntoa(addr);

    // Print IP address and port
    os << ipStr << ":" << client.port;

    return os;
}