#pragma once

#include <string>

enum StatementType {
    INVALID,
    SELECT,
    DELETE,
    INSERT
};

struct Statement {
    StatementType type = StatementType::INVALID;
    std::string st;
};
