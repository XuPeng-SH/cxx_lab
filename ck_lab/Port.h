#pragma once

#include <string>
#include <memory>
#include <assert.h>
#include <atomic>
#include <iostream>
#include <list>

struct IProcessor;
namespace MyDB {

struct Data {
    int val;
};

/* using DataPtr = std::shared_ptr<Data>; */

struct State {
    static std::uintptr_t GetUint(Data* data) {
        return reinterpret_cast<std::uintptr_t>(data);
    }

    static Data* GetPtr(std::uintptr_t data) {
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
            if ((GetUint(data_) & FLAGS_MASK) != 0) {
                assert(false);
            }
        }

        ~DataPtr() {
            delete GetPtr(GetUint(data_) & PTR_MASK);
        }

        DataPtr(DataPtr const &) : data_(new Data()) {}
        DataPtr& operator=(DataPtr const &) = delete;

        Data* operator->() const { return data_; }
        Data& operator*() const { return *data_; }
        Data* get() const { return data_; }
        explicit operator bool() const { return data_; }

        Data* Release() {
            Data* ret = nullptr;
            std::swap(ret, data_);
            return ret;
        }

        uintptr_t Swap(std::atomic<Data*>& value, std::uintptr_t flags, std::uintptr_t mask) {
            Data* expected = nullptr;
            Data* desired = GetPtr(flags | GetUint(data_));

            while (!value.compare_exchange_weak(expected, desired)) {
                desired = GetPtr((GetUint(expected) & FLAGS_MASK & (~mask)) | flags | GetUint(data_));
            }

            data_ = GetPtr(GetUint(expected) & PTR_MASK);
            return GetUint(expected) & FLAGS_MASK;
        }

    };

    std::atomic<Data*> data_;

    State() : data_(new Data()) {
        assert((GetUint(data_) & FLAGS_MASK) == 0);
    }

    ~State() {
        Data* desired = nullptr;
        Data* expected = nullptr;

        while (!data_.compare_exchange_weak(expected, desired));

        expected = GetPtr(GetUint(expected) & PTR_MASK);
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
        Data* desired = GetPtr(flags);

        while (!data_.compare_exchange_weak(expected, desired))
            desired = GetPtr((GetUint(expected) & FLAGS_MASK & (!mask)) | flags | (GetUint(expected) & PTR_MASK));
        return GetUint(expected) & FLAGS_MASK;
    }

    std::uintptr_t GetFlags() const {
        return GetUint(data_.load()) & FLAGS_MASK;
    }
};

using StatePtr = std::shared_ptr<State>;

struct Port {
    StatePtr state_ = nullptr;
    State::DataPtr data_;

    IProcessor* processor_ = nullptr;

    Port() {};
    explicit Port(IProcessor* processor) : processor_(processor) {}

    bool
    IsConnected() const { return state_ != nullptr; }
    bool
    HasData() const {
        return state_->GetFlags() & State::HAS_DATA;
    }

    IProcessor&
    IMutableProcessor() const { return *processor_; }
    IProcessor&
    MutableProcessor() { return *processor_; }
};

using PortPtr = std::shared_ptr<Port>;

struct OutputPort;
struct InputPort : public Port {
    OutputPort* output_port_ = nullptr;
    mutable bool is_finished_ = false;

    Data
    PullData(bool set_not_needed = false) {
        std::uintptr_t flags = 0;
        state_->Pull(data_, flags, set_not_needed);
        is_finished_ = flags & State::IS_FINISHED;

        return std::move(*data_);
    }

    bool
    IsFinished() const {
        if (is_finished_) return true;

        auto flags = state_->GetFlags();
        is_finished_ = (flags & State::IS_FINISHED) && ((flags & State::HAS_DATA) == 0);
        return is_finished_;
    }

    void
    SetNeeded() {
        // TODO
    }

    void
    SetNotNeeded() {
        state_->SetFlags(0, State::IS_NEEDED);
    }

    void
    Close() {
        is_finished_ = true;
    }

    void
    Reopen() {
        if (!IsFinished()) return;
        state_->SetFlags(0, State::IS_FINISHED);
        is_finished_ = false;
    }

    OutputPort&
    MutableOutputPort() {
        return *output_port_;
    }
    const OutputPort&
    ImmutableOutputPort() const {
        return *output_port_;
    }
};

struct OutputPort : public Port {
    InputPort* input_port_ = nullptr;

    void
    Push(Data data) {
        std::uintptr_t flags = 0;
        *data_ = std::move(data);
        state_->Push(data_, flags);
    }

    void
    Finish() {
        state_->SetFlags(State::IS_FINISHED, State::IS_FINISHED);
    }

    bool
    IsNeeded() const {
        return state_->GetFlags() & State::IS_NEEDED;
    }
    bool
    IsFinished() const {
        return state_->GetFlags() & State::IS_FINISHED;
    }
    bool
    CanPush() const {
        auto flags = state_->GetFlags();
        return (flags & State::IS_NEEDED) && (flags & State::HAS_DATA);
    }

    InputPort&
    MutableInputPort() {
        return *input_port_;
    }
    const InputPort&
    ImmutableInputPort() {
        return *input_port_;
    }
};

using InputPorts = std::list<InputPort>;
using OutputPorts = std::list<OutputPort>;

void
Connect(OutputPort& output, InputPort& input);

} // namespace MyDB
