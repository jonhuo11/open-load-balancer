#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "lb_udp.h"

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

using namespace std;

struct Config {
    int listenPort;
    int mode;
    int serviceCount;
    uint16_t *servicePorts;
    Config() : listenPort(0), mode(0), serviceCount(0), servicePorts(nullptr) {}
    ~Config() {
        delete[] servicePorts;
    }
};

ostream &operator<<(std::ostream &out, const Config &val) {
    out << "Listen port: " << val.listenPort << "\nMode: " << val.mode << "\nClients: " << val.serviceCount << "\n";
    return out;
}

int main(int argc, char *argv[]) {
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

    // 0 is round robin
    // 1 is IP hashing
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

    // open client sockets for writing
    vector<unique_ptr<ServiceSocketUDP>> services(config.serviceCount);
    for (int i = 0; i < config.serviceCount; i++) {
        services[i] = make_unique<ServiceSocketUDP>("127.0.0.1", config.servicePorts[i]);
        cout << "Opened client socket at " << config.servicePorts[i] << endl;
    }

    // listen on socket for events
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        cerr << "Error creating socket" << endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;                      // IPv4
    serverAddr.sin_port = htons(config.listenPort);       // Port number
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost IP address

    if (bind(udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error binding socket" << endl;
        close(udpSocket);
        return 1;
    }

    cout << "\nLoad balancer started" << endl;

    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        cerr << "Error creating epoll" << endl;
        close(udpSocket);
        return 1;
    }

    struct epoll_event event, events[MAX_EVENTS];
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    event.events = EPOLLIN;
    event.data.fd = udpSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, udpSocket, &event) == -1) {
        cerr << "epoll_ctl: udp_socket" << endl;
        close(udpSocket);
        close(epollFd);
        return 1;
    }

    int sendClient = 0;
    for (;;) {
        int nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);  // number of events
        if (nfds == -1) {
            cerr << "epoll_wait failed" << endl;
            close(udpSocket);
            close(epollFd);
            return 1;
        }

        // does not execute if nfds < 1
        for (int i = 0; i < nfds; ++i) {
            if (events[i].events && EPOLLIN) {  // Data is available to read
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t n = recvfrom(udpSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientLen);
                if (n < 0) {
                    cerr << "recv failed" << endl;
                } else {
                    // TODO: migrate into lb.cc
                    if (sendClient >= services.size()) {
                        sendClient = 0;
                    }  // simple round robin schedule
                    services[sendClient]->send(buffer);
                    sendClient++;
                }
            }
        }
    }

    close(udpSocket);
    close(epollFd);
    return 0;
}