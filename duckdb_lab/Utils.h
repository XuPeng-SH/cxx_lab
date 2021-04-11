#pragma once

#include <algorithm>
#include <random>
#include <mutex>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <chrono>
#include "types.h"

/* template <typename T> */
/* T RandomNumber(T start, T end) { */
/*     std::random_device dev; */
/*     std::mt19937 rng(dev()); */
/*     std::uniform_int_distribution<std::mt19937::result_type> dist(start, end); */
/*     return dist(rng); */
/* } */

template <typename T>
T RandomNumber(const T & start, const T & end) {
    std::chrono::time_point<std::chrono::system_clock> now =
    std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto micro = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    return start + (micro % (end + 1 - start));
}

inline std::string
CurrentDateTimeString() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::move(ss.str());
}

class RNGenerator {
 public:
    enum class IDType : uint16_t {
        LAST_NAME = 255,
        CUSTOMER = 1023,
        ITEM = 8191
    };

    ID_TYPE
    Produce(IDType type, ID_TYPE start, ID_TYPE end);

    static RNGenerator&
    GetInstance() {
        static RNGenerator instance;
        return instance;
    }

 private:
    RNGenerator();

    ID_TYPE c_last_;
    ID_TYPE c_id_;
    ID_TYPE order_line_item_id_;
};

class SafeIDGenerator {
 public:
    static SafeIDGenerator&
    GetInstance() {
        static SafeIDGenerator instance;
        return instance;
    }

    SafeIDGenerator() = default;
    ~SafeIDGenerator() = default;

    ID_TYPE
    GetNextIDNumber();

    bool
    GetNextIDNumbers(size_t n, IDS_TYPE& ids);

 private:
    bool
    NextIDNumbers(size_t n, IDS_TYPE& ids);

    static constexpr size_t MAX_IDS_PER_MICRO = 1000;

    std::mutex mtx_;
    int64_t time_stamp_ms_ = 0;
};
