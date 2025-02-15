#include <fstream>
#include <iostream>

#include "config.h"
#include "udpLb.h"
#include "udpSockets.h"

using namespace std;

LoadBalancerUDP* g_loadBalancer;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        if (g_loadBalancer != nullptr) {
            g_loadBalancer->stop();
        }
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);

    // get destination ports from config.txt
    if (argc != 2) {
        cerr << "Usage: openloadbalancer [config file]" << endl;
        return 1;
    }

    ifstream configFile;
    configFile.open(argv[1]);
    if (!configFile) {
        cerr << "Could not find config file " << argv[1] << endl;
        return 1;
    }
    Config config;

    if (!(configFile >> config.listenPort)) {
        cerr << "Could not read listening port" << endl;
        return 1;
    }

    // 0 is random
    // 1 is IP hashing with round robin
    if (!(configFile >> config.mode)) {
        cerr << "Could not read load balance mode" << endl;
        return 1;
    }

    if (!(configFile >> config.serviceCount)) {
        cerr << "Could not read client count" << endl;
        return 1;
    }

    config.servicePorts = new uint16_t[config.serviceCount];
    for (int i = 0; i < config.serviceCount; i++) {
        int clientPort;
        if (!(configFile >> clientPort)) {
            cerr << "Could not read client " << i << "'s port number" << endl;
            return 1;
        }
        config.servicePorts[i] = clientPort;
    }
    

    configFile.close();
    cout << config << endl;

    // start load balancer
    auto lb = make_unique<LoadBalancerUDP>(config);
    g_loadBalancer = lb.get();
    try {
        lb->start();
    } catch (const exception& e) {
        cout << "An error occurred while shutting down: " << e.what() << endl;
    }
}