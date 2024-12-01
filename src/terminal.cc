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
    while (stream >> word) {
        tokens.push_back(word);
    }
    // Create a vector of const strings
    vector<string> constTokens(tokens.begin(), tokens.end());

    executeLine(constTokens);
    return *this;
}

void Terminal::executeLine(TerminalLineTokens& lineTokens) {
    if (lineTokens.size() < 1) throw LineExecutionError{};

    const string commandName = lineTokens[0];
    if (commandName == "quit") {
        lb.stop();
    } else {  // default behavior
        throw LineExecutionError{};
    }
}