#pragma once

#include <string>
#include <memory>
#include "doc.h"

struct Table {
    std::string name;
    uint64_t id;
    uint64_t current_segment_id;
};

using TablePtr = std::shared_ptr<Table>;
