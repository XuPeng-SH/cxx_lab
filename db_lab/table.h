#pragma once

#include <memory>
#include <string>
#include <sstream>

#include "status.h"
#include "pager.h"

struct Table : public std::enable_shared_from_this<Table> {
    uint32_t num_rows = 0;
    std::shared_ptr<Pager> pager = nullptr;
    static std::shared_ptr<Table>
    Open(const std::string& db_path) {
        auto pager = Pager::Open(db_path);
        /* if (!pager) { */
        /*     return nullptr; */
        /* } */
        auto table = std::make_shared<Table>();
        table->pager = pager;
        /* table->num_rows = pager->file_length / sizeof(UserSchema); */
        table->num_rows = pager->PageNums() * Pager::ROWS_PER_PAGE;
        return table;
    }

    std::string
    ToString() {
        std::stringstream ss;
        ss << "<Table: Rows=" << num_rows << ">";
        return ss.str();
    }

    void
    Close() {
        pager->Close();
    }

    struct Cursor {
        void*
        Value() {
            uint32_t page_num = row_num / Pager::ROWS_PER_PAGE;
            void* page;
            auto status = table->pager->GetPage(page_num, page);
            if (!status.ok()) {
                return nullptr;
            }
            uint32_t row_offset = row_num % Pager::ROWS_PER_PAGE;
            uint32_t byte_offset = row_offset * sizeof(UserSchema);
            return (char*)page + byte_offset;
        }

        Status
        Advance() {
            Status status;
            if (end_of_table) {
                status.type = StatusType::CURSOR_END_OF_FILE;
                status.err_msg = "CURSOR_END_OF_FILE";
                return status;
            }
            row_num += 1;
            if (row_num >= table->num_rows) {
                end_of_table = true;
            }
            return status;
        }

        std::shared_ptr<Table> table;
        uint32_t row_num;
        bool end_of_table;
    };

    void
    Append(UserSchema& row) {
        auto c = LastCursor();
        row.SerializeTo(c->Value());
        num_rows += 1;
    }

    std::shared_ptr<Cursor>
    StartCursor() {
        auto c = std::make_shared<Cursor>();
        c->table = shared_from_this();
        c->row_num = 0;
        c->end_of_table = (num_rows == 0);
        return c;
    }
    std::shared_ptr<Cursor>
    LastCursor() {
        auto c = std::make_shared<Cursor>();
        c->table = shared_from_this();
        c->row_num = num_rows;
        c->end_of_table = true;
        return c;
    }
};
