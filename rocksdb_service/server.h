#pragma once
#include <string>

class Server {
public:
    Server(const std::string& port);
    void run();

private:
    const std::string& port_;
};
