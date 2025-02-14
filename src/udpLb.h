#pragma once

#include <atomic>
#include <csignal>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <unordered_map>

#include "config.h"
#include "terminal.h"
#include "udpSockets.h"
#include "util.h"

using namespace std;

class Terminal;

class LoadBalancerUDP : private NonCopyableNonMovable {
    ServerSocketUDP socket;
    const Config& cfg;
    unordered_map<size_t, unique_ptr<ServiceSocketUDP>> services;
    atomic<bool> running;
    unique_ptr<Terminal> terminal;

   public:
    explicit LoadBalancerUDP(const Config& cfg);
    ~LoadBalancerUDP();
    void stop();
    void start();

   private:
    // TODO: these should be singletons
    class BalanceStrategy {
       protected:
        LoadBalancerUDP& lb;

       public:
        BalanceStrategy(LoadBalancerUDP& lb) : lb(lb) {};
        virtual void routePacket(PacketUDP& p) = 0;
        virtual ~BalanceStrategy() = default;

        // a new service is registered
        virtual void serviceUp(unique_ptr<ServiceSocketUDP>&&) = 0;
        // a service is brought down
        virtual void serviceDown(size_t downedServiceIndex) = 0;
    };

    // randomly pick a service and send the packet to it
    class RandomBalance : public BalanceStrategy {
        random_device rdev;
        mt19937 gen;
        uniform_int_distribution<> distr;

       public:
        RandomBalance(LoadBalancerUDP& lb);
        void routePacket(PacketUDP& p) override;
        void serviceUp(unique_ptr<ServiceSocketUDP>&&) override;
        void serviceDown(size_t) override;
    };

    // keep track of which clients are mapped to which services, based on IP and port hashing
    class RoundRobinServiceAssignmentBalance : public BalanceStrategy {
        size_t round_i = 0;                                        // which is the next server to assign to
        unordered_map<ClientIdentifier, size_t> clientToService;
        unordered_map<size_t, vector<ClientIdentifier>> serviceToClients;

       public:
        RoundRobinServiceAssignmentBalance(LoadBalancerUDP& lb);
        void routePacket(PacketUDP& p) override;
        void serviceUp(unique_ptr<ServiceSocketUDP>&&) override;
        void serviceDown(size_t) override;
    };

    unique_ptr<BalanceStrategy> balanceStrategy;
    void main(promise<void>&);
};
