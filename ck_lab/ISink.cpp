#include "ISink.h"

Status
ISink::Prepare(State& state) {
    if (state_ == State::Ready) {
        return Status::OK();
    }

    auto& input = inputs_.front();

    if (input.IsFinished() && state_ != State::Finished) {
        state_ = State::Finished;
        OnFinish();
        return Status::OK();
    }

    input.SetNeeded();
    if (!input.HasData()) {
        state_ = State::NeedData;
        return Status::OK();
    }

    chunk_ = input.Pull(true);
    state_ = State::Ready;

    return Status::OK();
}

Status
ISink::Work() {
    Status status;
    STATUS_CHECK(Consume(std::move(chunk_)));
    state_ = State::Initialized;
    return status;
}

Status
ISink::OnFinish() {
    return Status::OK();
}
