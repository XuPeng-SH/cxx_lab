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

    CHILD_OVERFLOW,

    EMPTY_KEY,
    KEY_OVERFLOW,

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

#define STATUS_CHECK(func)   \
    do {                     \
        status = func;       \
        if (!status.ok()) {  \
            return  status;  \
        }                    \
    } while (false)

/* #define STATUS_CHECK(func) \ */
/*     do {                   \ */
/*         Status s = func;   \ */
/*         if (!s.ok()) {     \ */
/*             return s;      \ */
/*         }                  \ */
/*     } while (false) */
