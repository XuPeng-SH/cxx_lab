#include "Status.h"
#include <memory>

#include <cstring>

constexpr int CODE_WIDTH = sizeof(StatusCode);

Status::Status(StatusCode code, const std::string& msg) {
    // 4 bytes store code
    // 4 bytes store message length
    // the left bytes store message string
    auto length = static_cast<uint32_t>(msg.size());
    // auto result = new char[length + sizeof(length) + CODE_WIDTH];
    state_.resize(length + sizeof(length) + CODE_WIDTH);
    std::memcpy(state_.data(), &code, CODE_WIDTH);
    std::memcpy(state_.data() + CODE_WIDTH, &length, sizeof(length));
    memcpy(state_.data() + sizeof(length) + CODE_WIDTH, msg.data(), length);
}

Status::~Status() {
}

Status::Status(const Status& s) {
    CopyFrom(s);
}

Status::Status(Status&& s) noexcept {
    MoveFrom(s);
}

Status&
Status::operator=(const Status& s) {
    CopyFrom(s);
    return *this;
}

Status&
Status::operator=(Status&& s) noexcept {
    MoveFrom(s);
    return *this;
}

void
Status::CopyFrom(const Status& s) {
    state_.clear();
    if (s.state_.empty()) {
        return;
    }

    uint32_t length = 0;
    memcpy(&length, s.state_.data() + CODE_WIDTH, sizeof(length));
    int buff_len = length + sizeof(length) + CODE_WIDTH;
    state_.resize(buff_len);
    memcpy(state_.data(), s.state_.data(), buff_len);
}

void
Status::MoveFrom(Status& s) {
    state_ = s.state_;
    s.state_.clear();
}

std::string
Status::message() const {
    if (state_.empty()) {
        return "OK";
    }

    std::string msg;
    uint32_t length = 0;
    memcpy(&length, state_.data() + CODE_WIDTH, sizeof(length));
    if (length > 0) {
        msg.append(state_.data() + sizeof(length) + CODE_WIDTH, length);
    }

    return msg;
}

std::string
Status::ToString() const {
    if (state_.empty()) {
        return "OK";
    }

    std::string result;
    switch (code()) {
        case 0:
            result = "OK ";
            break;
        default:
            result = "Error code(" + std::to_string(code()) + "): ";
            break;
    }

    result += message();
    return result;
}
