#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <assert.h>

#include "status.h"
#include "pager.h"
#include "node.h"
#include "user_schema.h"

struct Table : public std::enable_shared_from_this<Table> {
    uint32_t root_page_num;
    std::shared_ptr<Pager> pager = nullptr;
    static std::shared_ptr<Table>
    Open(const std::string& db_path) {
        auto pager = Pager::Open(db_path);
        auto table = std::make_shared<Table>();
        table->pager = pager;
        table->root_page_num = 0;
        if (pager->num_pages == 0) {
            void* page;
            auto status = pager->GetPage(0, page);
            assert(status.ok());
            LeafPage* leaf = new (page) LeafPage();
            leaf->Reset();
            leaf->SetRoot(true);
        }
        return table;
    }

    uint32_t
    NumOfPages() const {
        return pager->num_pages;
    }

    std::string
    ToString() {
        std::stringstream ss;
        ss << "<Table: RootPageNum=" << root_page_num << " PageNums=" << pager->num_pages << ">";
        return ss.str();
    }

    Status
    SplitRoot(uint32_t right_child_page_num) {
        Status status;
        auto left_child_page_num = pager->PageNums();
        void* root;
        void* left_child;
        void* right_child;
        STATUS_CHECK(pager->GetPage(root_page_num, root));
        STATUS_CHECK(pager->GetPage(left_child_page_num, left_child));
        STATUS_CHECK(pager->GetPage(right_child_page_num, right_child));

        InternalPage* internal_root = (InternalPage*)root;
        LeafPage* left_leaf = (LeafPage*)left_child;
        LeafPage* right_leaf = (LeafPage*)right_child;

        memcpy(left_child, root, Pager::PAGE_SIZE);
        left_leaf->SetRoot(false);

        internal_root->Reset();
        internal_root->SetRoot(true);
        internal_root->SetNumOfKeys(1);
        internal_root->SetChild(0, left_child_page_num);
        uint32_t left_max_key;
        STATUS_CHECK(left_leaf->GetMaxKey(left_max_key));
        STATUS_CHECK(internal_root->SetKey(0, left_max_key));
        internal_root->SetRightChild(right_child_page_num);

        left_leaf->SetParentPage(root_page_num);
        right_leaf->SetParentPage(root_page_num);

        return status;
    }

    void
    Close() {
        pager->Close();
    }

    struct Cursor {
        void*
        Value() {
            LeafPage* leaf;
            auto status = GetLeafPage(leaf);
            if (!status.ok()) {
                return nullptr;
            }
            return leaf->CellValPtr(cell_num);
        }

        Status
        GetLeafPage(LeafPage*& leaf, uint32_t num) {
            void* page;
            auto status = table->pager->GetPage(num, page);
            if (!status.ok()) {
                return status;
            }
            leaf = new (page) LeafPage();
            return status;
        }

        Status
        GetLeafPage(LeafPage*& leaf) {
            return GetLeafPage(leaf, page_num);
        }

        // TODO
        Status
        DoInsertAndSplit(uint32_t curr_page_num, LeafPage* curr_page, const uint32_t& key, UserSchema& row) {
            // Stage 1
            auto next_page_num = table->pager->PageNums();
            LeafPage* next_page;
            Status status = GetLeafPage(next_page, next_page_num);
            if (!status.ok()) {
                return status;
            }
            next_page->SetParentPage(curr_page->GetParentPage());
            next_page->SetNextLeaf(curr_page->GetNextLeaf());
            curr_page->SetNextLeaf(next_page_num);

            // Stage 2
            for (int32_t i = LeafPage::CellsCapacity; i >= 0; --i) {
                LeafPage* des_page;
                if (i >= LeafPage::LeftSplitCount) {
                    des_page = next_page;
                } else {
                    des_page = curr_page;
                }

                auto curr_cell_num = i % LeafPage::LeftSplitCount;
                auto curr_cell = des_page->CellPtr(curr_cell_num);
                if (!curr_cell) {
                    status.type = StatusType::CELL_OVERFLOW;
                    status.err_msg = "CELL_OVERFLOW";
                    return status;
                }
                if (i > this->cell_num) {
                    memcpy(curr_cell, curr_page->CellPtr(i-1), LeafPage::CellSize);
                } else if (i < this->cell_num) {
                    /* memcpy(curr_cell, curr_page->CellPtr(i)) */
                } else {
                    STATUS_CHECK(des_page->PutKey(i, key));
                    STATUS_CHECK(des_page->PutVal(i, row));
                }
            }

            curr_page->SetNumOfCells(LeafPage::LeftSplitCount);
            next_page->SetNumOfCells(LeafPage::RightSplitCount);

            if (curr_page->IsRoot()) {
                return table->SplitRoot(next_page_num);
            } else {
                void* page;
                STATUS_CHECK(table->pager->GetPage(curr_page->GetParentPage(), page));
                InternalPage* parent = (InternalPage*)(page);
                // TODO:
                //parent->UpdateKey();
                /* parent->Insert(); */
            }

            return status;
        }

        Status
        Insert(uint32_t key, UserSchema& row) {
            Status status;
            LeafPage* leaf = nullptr;
            status = GetLeafPage(leaf);
            if (!status.ok()) {
                return status;
            }
            if (leaf->NumOfCells() >= LeafPage::CellsCapacity) {
                status = DoInsertAndSplit(page_num, leaf, key, row);
                if (!status.ok()) {
                    return status;
                }
                return status;
            }

            if (cell_num < leaf->NumOfCells()) {
                for (uint32_t i = leaf->NumOfCells(); i > cell_num; --i) {
                    memcpy(leaf->CellPtr(i), leaf->CellPtr(i-1), LeafPage::CellSize);
                }
            }

            leaf->IncNumOfCells();
            status = leaf->PutKey(cell_num, key);
            if (!status.ok()) {
                return status;
            }
            status = leaf->PutVal(cell_num, row);

            return status;
        }

        Status
        Advance() {
            Status status;
            if (end_of_table) {
                status.type = StatusType::CURSOR_END_OF_FILE;
                status.err_msg = "CURSOR_END_OF_FILE";
                return status;
            }
            LeafPage* leaf;
            status = GetLeafPage(leaf);
            if (!status.ok()) {
                return status;
            }

            cell_num += 1;

            if (cell_num >= leaf->NumOfCells()) {
                auto next_page_num = leaf->GetNextLeaf();
                if (next_page_num == 0) {
                    end_of_table = true;
                } else {
                    page_num = next_page_num;
                    cell_num = 0;
                }
            }

            return status;
        }

        std::shared_ptr<Table> table;
        uint32_t page_num;
        uint32_t cell_num;
        bool end_of_table;
    };

    /* void */
    /* Append(UserSchema& row) { */
    /*     auto c = LastCursor(); */
    /*     row.SerializeTo(c->Value()); */
    /*     num_rows += 1; // TODO: Remove */
    /* } */

    std::shared_ptr<Cursor>
    StartCursor() {
        auto c = std::make_shared<Cursor>();
        c->table = shared_from_this();
        c->page_num = 0;

        LeafPage* leaf;
        auto status = c->GetLeafPage(leaf);
        if (!status.ok()) {
            return nullptr;
        }

        c->end_of_table = (leaf->NumOfCells() == 0);
        return c;
    }
    /* std::shared_ptr<Cursor> */
    /* LastCursor() { */
    /*     auto c = std::make_shared<Cursor>(); */
    /*     c->table = shared_from_this(); */
    /*     c->row_num = num_rows; // TODO: Remove */
    /*     c->end_of_table = true; */
    /*     return c; */
    /* } */
};
