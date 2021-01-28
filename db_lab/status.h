#pragma once

#include <string>

enum StatusType {
    INVALID_CMD,
    INVALID_ST,
    ILLIGLE_ST,

    PAGE_NUM_OVERFLOW,
    PAGE_LOAD_ERR,
    PAGE_FLUSH_ERR,

    CURSOR_END_OF_FILE,

    CELL_OVERFLOW,

    EMPTY_KEY,

    OK,
    EMPTY,
    EXIT
};

struct Status {
    StatusType type = StatusType::OK;
    std::string err_msg = "";
    bool
    ok() const { return type == StatusType::OK; }
};
