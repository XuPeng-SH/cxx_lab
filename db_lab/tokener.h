#pragma once

#include <string>
#include <vector>

class Tokener {
 public:
    static std::vector<std::string>
    Parse(const std::string& line) {
        std::vector<std::string> out;
        if (line.length() == 0) {
            return std::move(out);
        }
        size_t pos = 0;
        size_t pre_pos = 0;
        static const char* DELIMITER = " ";
        while ((pos = line.find(DELIMITER, pre_pos)) != std::string::npos) {
            auto l = line.substr(pre_pos, pos - pre_pos);
            /* cout << "xxx " << l << " pre_pos = " << pre_pos << " pos =" << pos << endl; */
            out.emplace_back(std::move(l));
            pre_pos = pos + 1;
        }
        auto l = line.substr(pre_pos, pos);
        if (l.length() != 0) {
            out.emplace_back(std::move(l));
        }
        return std::move(out);
    }
};
