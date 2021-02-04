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

    uint32_t
    GetRootPageNum() const {
        return root_page_num;
    }

    Node*
    GetRootPage() const {
        void* root;
        auto status = pager->GetPage(root_page_num, root);
        if (!status.ok()) {
            return nullptr;
        }
        return (Node*)root;
    }

    std::string
    ToString() {
        std::stringstream ss;
        ss << "<Table: RootPageNum=" << root_page_num << " PageNums=" << pager->num_pages << ">";
        return ss.str();
    }

    void
    DumpTree() {
        auto cursor = StartCursor();
        while(!cursor->end_of_table) {
            UserSchema row;
            row.DeserializeFrom(cursor->Value());
            std::cout << "[Page=" << cursor->page_num << ",CellNum=" << cursor->cell_num << "]: "  << row.ToString() << std::endl;
            cursor->Advance();
        }
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

        assert(internal_root->IsRoot());
        /* assert(internal_root->GetType() == Node::Type::LEAF); */

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
            if (!leaf->IsDirty()) {
                leaf->Reset();
            }
            return status;
        }

        Status
        GetLeafPage(LeafPage*& leaf) {
            return GetLeafPage(leaf, page_num);
        }

        Status
        InsertInternal(InternalPage* parent, void* child, uint32_t child_page_num) {
            Status status;
            uint32_t max_key;
            STATUS_CHECK(PageHelper::GetMaxKey(child, max_key));
            auto num_keys = parent->NumOfKeys();
            parent->SetNumOfKeys(num_keys + 1);
            uint32_t index = parent->FindCell(num_keys);
            if (num_keys >= InternalPage::MaxCells) {
                // TODO
                assert(false);
            }

            auto right_child_page = parent->RightChildPage();
            void* right_child;
            STATUS_CHECK(table->pager->GetPage(right_child_page, right_child));
            uint32_t right_child_max_key;
            STATUS_CHECK(PageHelper::GetMaxKey(right_child, right_child_max_key));
            if (max_key > right_child_max_key) {
                parent->SetKey(num_keys, right_child_max_key);
                parent->SetChild(num_keys, right_child_page);
                parent->SetRightChild(child_page_num);
            } else {
                for (uint32_t i = num_keys; i > index; --i) {
                    memcpy(parent->CellPtr(i), parent->CellPtr(i-1), InternalPage::CellSize);
                }
                STATUS_CHECK(parent->SetKey(index, max_key));
                STATUS_CHECK(parent->SetChild(index, child_page_num));
            }

            return status;
        }

        Status
        DoInsertAndSplit(uint32_t curr_page_num, LeafPage* curr_page, const uint32_t& key, UserSchema& row) {
            /* std::cout << __func__ << ": curr_page_num=" << curr_page_num << " key=" << key << std::endl; */
            // Stage 1
            auto next_page_num = table->pager->PageNums();
            LeafPage* next_page;
            Status status = GetLeafPage(next_page, next_page_num);
            if (!status.ok()) {
                return status;
            }

            uint32_t old_max;
            STATUS_CHECK(curr_page->GetMaxKey(old_max));
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
                    memcpy(curr_cell, curr_page->CellPtr(i), LeafPage::CellSize);
                } else {
                    /* std::cout << "**** " << curr_cell_num << ", " << key << ", " << "curr_page_num=" << curr_page_num << std::endl; */
                    STATUS_CHECK(des_page->PutKey(curr_cell_num, key));
                    STATUS_CHECK(des_page->PutVal(curr_cell_num, row));
                }
            }

            curr_page->SetNumOfCells(LeafPage::LeftSplitCount);
            next_page->SetNumOfCells(LeafPage::RightSplitCount);

            if (curr_page->IsRoot()) {
                return table->SplitRoot(next_page_num);
            } else {
                void* page;
                STATUS_CHECK(table->pager->GetPage(curr_page->GetParentPage(), page));
                uint32_t new_max;
                STATUS_CHECK(PageHelper::GetMaxKey(page, new_max));
                InternalPage* parent = PageHelper::AsInternal(page);
                assert(parent);
                auto old_max_cell = parent->FindCell(old_max);
                STATUS_CHECK(parent->SetKey(old_max_cell, new_max));
                STATUS_CHECK(InsertInternal(parent, next_page, next_page_num));
            }

            return status;
        }

        Status
        ExecuteInsert(uint32_t key, UserSchema& row) {
            /* std::cout << __func__ << ": page_num=" << page_num << " key=" << key << std::endl; */
            Status status;

            LeafPage* leaf;
            STATUS_CHECK(GetLeafPage(leaf));
            if (leaf->NumOfCells() >= LeafPage::CellsCapacity) {
                status = DoInsertAndSplit(page_num, leaf, key, row);
                return status;
            }

            if (cell_num < leaf->NumOfCells()) {
                for (uint32_t i = leaf->NumOfCells(); i > cell_num; --i) {
                    memcpy(leaf->CellPtr(i), leaf->CellPtr(i-1), LeafPage::CellSize);
                }
            }

            leaf->IncNumOfCells();
            STATUS_CHECK(leaf->PutKey(cell_num, key));
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

    Status
    LeafPageFind(LeafPage* page, uint32_t key, std::shared_ptr<Cursor> cursor) {
        Status status;
        auto cell = page->FindCell(key);
        cursor->cell_num = cell;
        /* std::cout << __func__ << ": key=" << key << " cell_num=" << cell << std::endl; */
        return status;
    }

    Status
    InternalPageFind(InternalPage* page, uint32_t key, std::shared_ptr<Cursor> cursor) {
        auto cell_num = page->FindCell(key);
        uint32_t page_num;
        /* std::cout << __func__ << ": key=" << key << " cell_num=" << cell_num << std::endl; */
        Status status;
        STATUS_CHECK(page->GetChildPageNum(cell_num, page_num));
        void* sub_page;
        STATUS_CHECK(pager->GetPage(page_num, sub_page));
        LeafPage* lpage = PageHelper::AsLeaf(sub_page);
        if (lpage) {
            cursor->page_num = page_num;
            return LeafPageFind(lpage, key, cursor);
        }
        InternalPage* ipage = PageHelper::AsInternal(sub_page);
        if (!ipage) {
            status.type = StatusType::PAGE_INVALID;
            return status;
        }
        return InternalPageFind(ipage, key, cursor);
    }

    std::shared_ptr<Cursor>
    Find(uint32_t key) {
        void* page;
        auto status = pager->GetPage(root_page_num, page);
        if (!status.ok()) {
            return nullptr;
        }
        auto cursor = std::make_shared<Cursor>();
        cursor->table = shared_from_this();
        cursor->page_num = root_page_num;
        cursor->end_of_table = false;
        InternalPage* ipage = PageHelper::AsInternal(page);
        if (ipage) {
            status = InternalPageFind(ipage, key, cursor);
            if (!status.ok()) {
                return nullptr;
            }
            return cursor;
        }

        LeafPage* lpage = PageHelper::AsLeaf(page);
        if (!lpage) {
            return nullptr;
        }

        status = LeafPageFind(lpage, key, cursor);
        if (!status.ok()) {
            return nullptr;
        }
        return cursor;
    }

    Status
    Insert(uint32_t key, UserSchema& row) {
        Status status;
        auto cursor = Find(key);
        /* std::cout << __func__ << ":" << __LINE__ << " cursor->page_num=" << cursor->page_num << std::endl; */
        if (!cursor) {
            status.type = StatusType::KEY_OVERFLOW;
            return status;
        }

        status = cursor->ExecuteInsert(key, row);
        return status;
    }

    std::shared_ptr<Cursor>
    StartCursor() {
        auto c = Find(0);

        LeafPage* leaf;
        auto status = c->GetLeafPage(leaf);
        if (!status.ok()) {
            return nullptr;
        }

        c->end_of_table = (leaf->NumOfCells() == 0);
        return c;
    }
};
