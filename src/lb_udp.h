#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

using namespace std;

// services that the load balancer is distributing traffic across
class ServiceSocketUDP {
    int socket;
    struct sockaddr_in destAddr;

   public:
    ServiceSocketUDP(const char* ip, uint16_t port);
    ~ServiceSocketUDP();
    int send(const char* message);
};

// socket that server reads incoming udp traffic from
class ServerSocketUDP {
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

// load balancer
class LoadBalancerUDP {
    const vector<unique_ptr<ServiceSocketUDP>>& services;

   public:
    LoadBalancerUDP(const vector<unique_ptr<ServiceSocketUDP>>& services, unsigned int balanceStrategy);

   private:
    class BalanceStrategy {
       protected:
        LoadBalancerUDP& lb;

       public:
        BalanceStrategy(LoadBalancerUDP& lb) : lb(lb) {};
        virtual size_t calculateDestinationServiceForPacket(Packet& p) = 0;  // returns the index of the service socket to send packet to
        virtual ~BalanceStrategy() = default;
    };

    // randomly pick a service and send the packet to it
    class RandomBalance : public BalanceStrategy {
        random_device rdev;
        mt19937 gen;
        uniform_int_distribution<> distr;

       public:
        RandomBalance(LoadBalancerUDP& lb);
        size_t calculateDestinationServiceForPacket(Packet& p) override;
    };

    // keep track of which clients are mapped to which services
    class RoundRobinServiceAssignmentBalance : public BalanceStrategy {
        size_t round_i = 0;  // which is the next server to assign to
        unordered_map<ClientIdentifier, size_t> clientServiceMap; // TODO: if a service goes down, the mappings need to be updated

       public:
        size_t calculateDestinationServiceForPacket(Packet& p) override;
    };

    unique_ptr<BalanceStrategy> balanceStrategy;
};
