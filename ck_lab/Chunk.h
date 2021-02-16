#pragma once

#include <vector>
#include "Status.h"

/// TODO
class Chunk {
 public:
     using FieldType = int;
     using RowType = std::vector<FieldType>;

     Status
     AddRow(const RowType& row) {
        rows_.emplace_back(row);
        return Status::OK();
     }

 private:
     std::vector<RowType> rows_;
};
