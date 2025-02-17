#pragma once

#include <atomic>
#include <csignal>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <unordered_map>
#include <shared_mutex>
#include <sstream>
#include <iterator>

#include "simpleOrderedDict.h"
#include "config.h"
#include "terminal.h"
#include "udpSockets.h"
#include "util.h"

using namespace std;

class Terminal;

class LoadBalancerUDP : private NonCopyableNonMovable {
    ServerSocketUDP socket;
    const Config& cfg;
    SimpleOrderedDict<size_t, unique_ptr<ServiceSocketUDP>> services;
    atomic<bool> running;
    unique_ptr<Terminal> terminal;
    mutable shared_mutex mutex;

public:
    explicit LoadBalancerUDP(const Config& cfg);
    ~LoadBalancerUDP();
    void stop();
    void start();
    const ServerSocketUDP& getSocket() const;
    bool isRunning() const;
    void routePacket(const PacketUDP& packet);
    string listServices() const;
    void serviceUp(const char* ip, uint16_t port);
    void serviceDown (const size_t& serviceKey);

private:
    class BalanceStrategy {
    protected:
        const LoadBalancerUDP& lb;

    public:
        BalanceStrategy(const LoadBalancerUDP& lb) : lb(lb) {};
        virtual void routePacket(const PacketUDP& p) = 0;
        virtual ~BalanceStrategy() = default;

        // a new service is registered
        virtual void serviceUp(size_t serviceKey) = 0;
        // a service is brought down
        virtual void serviceDown(size_t downedServiceIndex) = 0;
    };

    // randomly pick a service and send the packet to it
    class RandomBalance : public BalanceStrategy {
        random_device rdev;
        mt19937 gen;
        uniform_int_distribution<> distr;

    public:
        RandomBalance(const LoadBalancerUDP& lb);
        void routePacket(const PacketUDP& p) override;
        void serviceUp(size_t serviceKey) override;
        void serviceDown(size_t) override;
    };

    // keep track of which clients are mapped to which services, based on IP and port hashing
    class RoundRobinBalance : public BalanceStrategy {
        class ServiceKeyManager {
            const RoundRobinBalance& balancer;
            size_t currentReadyServiceKey = 0;                                        // which is the next server to assign to
            size_t nextReadyServiceKey = 0;
            
        public:
            ServiceKeyManager(const RoundRobinBalance&);
            size_t getCurrentReadyServiceKey() const;
            size_t getNextReadyServiceKey() const;
            void updateCurrentReadyServiceKey();
            void updateNextReadyServiceKey();
            void setCurrentReadyServiceKeyToLastServiceKey();
        };

        ServiceKeyManager skm;

        // TODO: avoid making copy of the ClientIdentifier struct
        unordered_map<ClientIdentifier, size_t> clientToService;
        unordered_map<size_t, vector<ClientIdentifier>> serviceToClients;

    public:
        RoundRobinBalance(const LoadBalancerUDP& lb);
        void routePacket(const PacketUDP& p) override;
        void serviceUp(size_t serviceKey) override;
        void serviceDown(size_t) override;

    };

private:
    unique_ptr<BalanceStrategy> balanceStrategy;
};

void loadBalancerSingleThreaded(LoadBalancerUDP& lb, promise<void>&);
