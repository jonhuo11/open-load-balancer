#pragma once

#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

#include "config.h"
#include "udpSockets.h"
#include "util.h"

using namespace std;

class LoadBalancerUDP : private NonCopyableNonMovable {
    ServerSocketUDP socket;
    const Config& cfg;
    const vector<unique_ptr<ServiceSocketUDP>>& services;

   public:
    LoadBalancerUDP(const Config& cfg, const vector<unique_ptr<ServiceSocketUDP>>& services, unsigned int balanceStrategy);
    void main();

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
        size_t round_i = 0;                                        // which is the next server to assign to
        unordered_map<ClientIdentifier, size_t> clientServiceMap;  // TODO: if a service goes down, the mappings need to be updated

       public:
        size_t calculateDestinationServiceForPacket(Packet& p) override;
    };

    unique_ptr<BalanceStrategy> balanceStrategy;
};
