#pragma once

#include "node.h"
#include "table.h"
#include "status.h"
#include "user_schema.h"

struct LeafPageOperator {
    LeafPageOperator(LeafPage* page) : leaf(page) {}

    Status
    Insert(uint32_t key, UserSchema& row) {

    }

    LeafPage* leaf;
};
