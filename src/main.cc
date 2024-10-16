#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <vector>
#include <memory>

#include "sockets.cc"

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

using namespace std;

struct Config {
    int listen_port;
    int mode;
    int client_count;
    uint16_t* client_ports;
    Config(): listen_port(0), mode(0), client_count(0), client_ports(nullptr) {}
    ~Config() {
        delete[] client_ports;
    }
};

ostream& operator<<(std::ostream& out, const Config& val){
    out << "Listen port: " << val.listen_port << "\nMode: " << val.mode << "\nClients: " << val.client_count << "\n";
    return out;
}


int main(int argc, char* argv[]) {

    // get destination ports from config.txt
    if (argc != 2) {
        cerr << "Usage: openloadbalancer [config file]" << endl;
        return 1;
    }

    ifstream config_file;
    config_file.open(argv[1]);
    if (!config_file) {
        cerr << "Could not find config file " << argv[1] << endl;
        return 1;
    }
    Config config;

    if (!(config_file >> config.listen_port)) {
        cerr << "Could not read listening port" << endl;
        return 1;
    }

    // 0 is round robin
    // 1 is IP hashing
    if (!(config_file >> config.mode)) {
        cerr << "Could not read load balance mode" << endl;
        return 1;
    }

    if (!(config_file >> config.client_count)) {
        cerr << "Could not read client count" << endl;
        return 1;
    }

    config.client_ports = new uint16_t[config.client_count];
    for (int i = 0; i < config.client_count; i++) {
        int client_port;
        if (!(config_file >> client_port)) {
            cerr << "Could not read client " << i << "'s port number" << endl;
            return 1;
        }
        config.client_ports[i] = client_port;
    }

    config_file.close();

    cout << config << endl;

    // open client sockets for writing
    vector<unique_ptr<ClientSocketUDP>> clients(config.client_count);    
    for (int i = 0; i < config.client_count; i++) {
        clients[i] = make_unique<ClientSocketUDP>("127.0.0.1", config.client_ports[i]);
        cout << "Opened client socket at " << config.client_ports[i] << endl;
    }


    // listen on socket for events
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        cerr << "Error creating socket" << endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                       // IPv4
    server_addr.sin_port = htons(config.listen_port);                     // Port number
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   // Localhost IP address

    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Error binding socket" << endl;
        close(udp_socket);
        return 1;
    }

    cout << "\nLoad balancer started" << endl;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        cerr << "Error creating epoll" << endl;
        close(udp_socket);
        return 1;
    }

    struct epoll_event event, events[MAX_EVENTS];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    event.events = EPOLLIN;
    event.data.fd = udp_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_socket, &event) == -1) {
        cerr << "epoll_ctl: udp_socket" << endl;
        close(udp_socket);
        close(epoll_fd);
        return 1;
    }

    int send_client = 0;
    for (;;) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // nubmer of events
        if (nfds == -1) {
            cerr << "epoll_wait failed" << endl;
            close(udp_socket);
            close(epoll_fd);
            return 1;
        }

        // does not execute if nfds < 1
        for (int i = 0; i < nfds; ++i) {
            if (events[i].events & EPOLLIN) { // Data is available to read
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t n = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
                if (n < 0) {
                    cerr << "recv failed" << endl;
                } else {

                    // TODO: migrate into lb.cc
                    if (send_client >= clients.size()) { send_client = 0; } // simple round robin schedule
                    clients[send_client]->send(buffer);
                    send_client++;
                }
            }
        }
    }


    close(udp_socket);
    close(epoll_fd);
    return 0;
}