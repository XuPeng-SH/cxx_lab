#pragma once
#include <string>

class Server {
public:
    Server(const std::string& port, const std::string& db_path);
    void run();

private:
    const std::string& port_;
    const std::string& db_path_;
};
