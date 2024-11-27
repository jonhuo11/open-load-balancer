#pragma once

#include <ostream>

using namespace std;

struct Config {
    int listenPort;
    int mode;
    int serviceCount;
    uint16_t *servicePorts;
    Config();
    ~Config();
};

ostream &operator<<(std::ostream &out, const Config &val);