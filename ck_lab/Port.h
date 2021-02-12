#pragma once

#include <string>
#include <memory>
#include <assert.h>
#include <atomic>
#include <iostream>

struct Data {
    int val;
};

struct State {
    static std::uintptr_t getUint(Data* data) {
        return reinterpret_cast<std::uintptr_t>(data);
    }

    static Data* getPtr(std::uintptr_t data) {
        return reinterpret_cast<Data*>(data);
    }

    static constexpr std::uintptr_t IS_FINISHED = 0x1 << 0;
    static constexpr std::uintptr_t IS_NEEDED = 0x1 << 1;
    static constexpr std::uintptr_t HAS_DATA = 0x1 << 2;
    static constexpr std::uintptr_t FLAGS_MASK = IS_FINISHED | IS_NEEDED | HAS_DATA;
    static constexpr std::uintptr_t PTR_MASK = ~FLAGS_MASK;

    struct DataPtr {
        Data* data_ = nullptr;
        DataPtr() : data_(new Data()) {
            if ((getUint(data_) & FLAGS_MASK) != 0) {
                assert(false);
            }
        }

        ~DataPtr() {
            delete getPtr(getUint(data_) & PTR_MASK);
        }

        Data* Release() {
            Data* ret = nullptr;
            std::swap(ret, data_);
            return ret;
        }

        uintptr_t Swap(std::atomic<Data*>& value, std::uintptr_t flags, std::uintptr_t mask) {
            Data* expected = nullptr;
            Data* desired = getPtr(flags | getUint(data_));

            while (!value.compare_exchange_weak(expected, desired)) {
                desired = getPtr((getUint(expected) & FLAGS_MASK & (~mask)) | flags | getUint(data_));
            }

            data_ = getPtr(getUint(expected) & PTR_MASK);
            return getUint(expected) & FLAGS_MASK;
        }

    };

    std::atomic<Data*> data_;

    State() : data_(new Data()) {
        assert((getUint(data_) & FLAGS_MASK) == 0);
    }

    ~State() {
        Data* desired = nullptr;
        Data* expected = nullptr;

        while (!data_.compare_exchange_weak(expected, desired));

        expected = getPtr(getUint(expected) & PTR_MASK);
        delete expected;
    }

    void Push(DataPtr& data, std::uintptr_t& flags) {
        flags = data.Swap(data_, HAS_DATA, HAS_DATA);
        assert(!(flags & HAS_DATA));
    }

    void Pull(DataPtr& data, std::uintptr_t& flags, bool set_not_needed = false) {
        uintptr_t mask = HAS_DATA;

        if (set_not_needed) {
            mask |= IS_NEEDED;
        }

        flags = data.Swap(data_, 0, mask);

        assert(!((flags & IS_NEEDED) == 0 && !set_not_needed));
        assert(flags & HAS_DATA != 0);
    }

    std::uintptr_t
    SetFlags(std::uintptr_t flags, std::uintptr_t mask) {
        Data* expected;
        Data* desired = getPtr(flags);

        while (!data_.compare_exchange_weak(expected, desired))
            desired = getPtr((getUint(expected) & FLAGS_MASK & (!mask)) | flags | (getUint(expected) & PTR_MASK));
        return getUint(expected) & FLAGS_MASK;
    }

    std::uintptr_t GetFlags() const {
        return getUint(data_.load()) & FLAGS_MASK;
    }
};
