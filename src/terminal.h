#pragma once

#include <iostream>
#include <sstream>
#include <vector>

#include "udpLb.h"

class LoadBalancerUDP;

using namespace std;

using TerminalLineTokens = const vector<string>;

class Terminal {
    class LineExecutionError : public exception {};
    LoadBalancerUDP& lb;
    bool running;

    void executeLine(TerminalLineTokens&);

   public:
    Terminal(LoadBalancerUDP& lb);
    void start();
    void stop();

    Terminal& operator<<(const string& line);  // read input from the terminal
};