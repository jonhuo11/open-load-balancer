#include "config.h"

Config::~Config() {
    if (servicePorts != nullptr) delete[] servicePorts;
}

Config::Config() : listenPort(0), mode(0), serviceCount(0), servicePorts(nullptr) {}

ostream &operator<<(std::ostream &out, const Config &val) {
    out << "Listen port: " << val.listenPort << "\nMode: " << val.mode << "\nClients: " << val.serviceCount << "\n";
    return out;
}