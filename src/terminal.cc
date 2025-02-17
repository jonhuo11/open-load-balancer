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
    cout << "Shutting down terminal..." << endl;
}

Terminal& Terminal::operator<<(const std::string& line) {
    istringstream stream(line);
    string word;
    vector<string> tokens; // TODO: we can optimise this by not creating a new vector every time and just overwriting the old one
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
        //cout << "quitting lb done" << endl;
        stop();
        //cout << "quitting done" << endl;
    } else if (commandName == "help") {
        cout << helpText << endl;
    } else if (commandName == "service_list") {
        auto list = lb.listServices();
        for (const auto& service : list) {
            cout << service << endl;
        }
    } else if (commandName == "service_up") {
    } else if (commandName == "service_down") {
        if (lineTokens.size() != 2) { throw LineExecutionError{}; }
        lb.serviceDown(stoull(lineTokens[1]));
    } else {  // default behavior
        throw LineExecutionError{};
    }
}
