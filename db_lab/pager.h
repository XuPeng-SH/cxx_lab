#pragma once

#include <string>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "consts.h"
#include "status.h"
#include "user_schema.h"

struct Pager {
    constexpr static const uint32_t PAGE_SIZE = 4 * 1024;
    constexpr static const uint32_t MAX_PAGES = MAX_STORE_SIZE / PAGE_SIZE;
    constexpr static const uint32_t ROWS_PER_PAGE = PAGE_SIZE / sizeof(UserSchema);

    static std::shared_ptr<Pager>
    Open(const std::string& db_path) {
        auto fd = open(db_path.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
        if (fd == -1) {
            std::cerr << "Cannot open \"" << db_path << "\"" << std::endl;
            return nullptr;
        }
        auto pager = std::make_shared<Pager>();
        pager->file_descriptor = fd;
        pager->file_length = lseek(pager->file_descriptor, 0, SEEK_END);
        pager->num_pages = pager->file_length / PAGE_SIZE;
        memset(pager->pages, 0, MAX_PAGES * sizeof(void*));
        return pager;
    }

    void
    Close() {
        Status status;
        for (auto num = 0; num < MAX_PAGES; ++num) {
            if (pages[num] == nullptr) {
                continue;
            }
            status = FlushPage(num);
            if (!status.ok()) {
                std::cerr << status.err_msg << std::endl;
                continue;
            }
            free(pages[num]);
            pages[num] = nullptr;
        }
    }

    Status
    OnPageMissing(const uint32_t& num) {
        Status status;
        pages[num] = calloc(PAGE_SIZE, 1);
        std::cout << "malloc new page " << num << std::endl;
        /* auto all_page_num = file_length / PAGE_SIZE; */
        if (num < num_pages) {
            lseek(file_descriptor, num * PAGE_SIZE, SEEK_SET);
            auto read_size = read(file_descriptor, pages[num], PAGE_SIZE);
            if (read_size == -1) {
                status.type = StatusType::PAGE_LOAD_ERR;
                status.err_msg = std::string("PAGE_LOAD_ERR: load page ") + std::to_string(num);
                return status;
            }
        }
        if (num >= num_pages) {
            num_pages = num + 1;
        }

        return status;
    }

    Status
    FlushPage(uint32_t num) {
        Status status;
        if (num > MAX_PAGES) {
            status.type = PAGE_NUM_OVERFLOW;
            status.err_msg = std::string("PAGE_NUM_OVERFLOW: ") + std::to_string(num);
            return status;
        }

        if (pages[num] == nullptr) {
            return status;
        }

        lseek(file_descriptor, num * PAGE_SIZE, SEEK_SET);
        auto written = write(file_descriptor, pages[num], PAGE_SIZE);
        if (written == -1) {
            status.type = PAGE_FLUSH_ERR;
            status.err_msg = std::string("PAGE_FLUSH_ERR: ") + std::to_string(num);
            return status;
        }
        std::cout << "page " << num << " flushed" << std::endl;
        return status;
    }

    Status
    GetPage(uint32_t num, void*& page) {
        Status status;
        if (num > MAX_PAGES) {
            status.type = StatusType::PAGE_NUM_OVERFLOW;
            status.err_msg = std::string("PAGE_NUM_OVERFLOW: ") + std::to_string(num);
        }

        if (pages[num] == nullptr) {
            status = OnPageMissing(num);
        }

        if (!status.ok()) {
            return status;
        }

        page = pages[num];

        return status;
    }

    uint32_t
    PageNums() const {
        return file_length / PAGE_SIZE;
    }

    ~Pager() {
        Close();
        if (file_descriptor != -1) {
            close(file_descriptor);
            file_descriptor = -1;
        }
    }

    int file_descriptor = -1;
    uint32_t file_length = 0;
    uint32_t num_pages = 0;
    void* pages[MAX_PAGES];
};
