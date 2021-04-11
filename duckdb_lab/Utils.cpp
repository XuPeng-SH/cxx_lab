#include "Utils.h"
#include <chrono>
#include <assert.h>

int64_t
SafeIDGenerator::GetNextIDNumber() {
    auto now = std::chrono::system_clock::now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    std::lock_guard<std::mutex> lock(mtx_);
    if (micros <= time_stamp_ms_) {
        time_stamp_ms_ += 1;
    } else {
        time_stamp_ms_ = micros;
    }
    return time_stamp_ms_ * MAX_IDS_PER_MICRO;
}

bool
SafeIDGenerator::GetNextIDNumbers(size_t n, std::vector<int64_t>& ids) {
    ids.clear();
    std::lock_guard<std::mutex> lock(mtx_);
    while (n > 0) {
        if (n > MAX_IDS_PER_MICRO) {
            auto status = NextIDNumbers(MAX_IDS_PER_MICRO, ids);
            if (!status) {
                return status;
            }
            n -= MAX_IDS_PER_MICRO;
        } else {
            auto status = NextIDNumbers(n, ids);
            if (!status) {
                return status;
            }
            break;
        }
    }
    return true;
}

bool
SafeIDGenerator::NextIDNumbers(size_t n, std::vector<int64_t>& ids) {
    if (n <= 0 || n > MAX_IDS_PER_MICRO) {
        std::string msg = "Invalid ID number: " + std::to_string(n);
        return false;
    }

    auto now = std::chrono::system_clock::now();
    int64_t micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    if (micros <= time_stamp_ms_) {
        time_stamp_ms_ += 1;
    } else {
        time_stamp_ms_ = micros;
    }

    int64_t ID_high_part = time_stamp_ms_ * MAX_IDS_PER_MICRO;

    for (size_t pos = 0; pos < n; ++pos) {
        ids.push_back(ID_high_part + pos);
    }

    return true;
}

RNGenerator::RNGenerator() {
    c_id_ = RandomNumber<ID_TYPE>(0, 255);
    c_last_ = RandomNumber<ID_TYPE>(0, 1023);
    order_line_item_id_ = RandomNumber<ID_TYPE>(0, 8191);
}

ID_TYPE
RNGenerator::Produce(IDType type, ID_TYPE start, ID_TYPE end) {
    assert(end >= start);
    ID_TYPE c = 0;
    if (type == IDType::LAST_NAME) {
        c = c_last_;
    } else if (type == IDType::CUSTOMER) {
        c = c_id_;
    } else if (type == IDType::ITEM) {
        c = order_line_item_id_;
    }
    auto part1 = RandomNumber<ID_TYPE>(0, (ID_TYPE)type);
    auto part2 = RandomNumber<ID_TYPE>(start, end);
    return ((part1 | part2) + c) % (end - start + 1) + start;
}
