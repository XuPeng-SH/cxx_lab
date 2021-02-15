#pragma once

#include "Error.h"

#include <string>

class Status;
#define STATUS_CHECK(func) \
    do {                   \
        if (!status.ok()) {     \
            return status;      \
        }                  \
    } while (false)

using StatusCode = ErrorCode;

class Status {
 public:
    Status(StatusCode code, const std::string& msg);
    Status() = default;
    virtual ~Status();

    Status(const Status& s);

    Status(Status&& s) noexcept;

    Status&
    operator=(const Status& s);

    Status&
    operator=(Status&& s) noexcept;

    static Status
    OK() {
        return Status();
    }

    bool
    ok() const {
        return state_.empty() || code() == 0;
    }

    StatusCode
    code() const {
        return (state_.empty()) ? 0 : *(StatusCode*)(state_.data());
    }

    std::string
    message() const;

    std::string
    ToString() const;

 private:
    inline void
    CopyFrom(const Status& s);

    inline void
    MoveFrom(Status& s);

 private:
    std::string state_;
};
