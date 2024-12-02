#include "terminal.h"

#include "udpLb.h"

Terminal::Terminal(LoadBalancerUDP& lb) : lb(lb), running(false) {
}

void Terminal::start() {
    running = true;
    string line;
    for (;;) {
        cout << ">>> ";
        if (!(running && getline(cin, line))) break;
        try {
            *this << line;
        } catch (LineExecutionError& e) {
            cout << "Failed to run command" << endl;
        }
    }
}

void Terminal::stop() {
    running = false;
}

Terminal& Terminal::operator<<(const std::string& line) {
    istringstream stream(line);
    string word;
    vector<string> tokens;
    while (stream >> word) tokens.push_back(word);
    executeLine(tokens);
    return *this;
}

void Terminal::executeLine(TerminalLineTokens& lineTokens) {
    if (lineTokens.size() < 1) throw LineExecutionError{};

    const char* helpText = R"(
Commands:
quit
help
service_list
    list services that are attached to the load balancer
service_down [service_number]
    bring a service down
service_up [ip] [port]
    register a new service
service_health [service_number]
    check the health of a specific service
)";

    const string commandName = lineTokens[0];
    if (commandName == "quit") {
        lb.stop();
    } else if (commandName == "help") {
        cout << helpText << endl;
    } else if (commandName == "service_list") {
    } else if (commandName == "service_down") {
    } else {  // default behavior
        throw LineExecutionError{};
    }
}
