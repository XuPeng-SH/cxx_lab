#include "ISink.h"

ISink::ISink() : input_(inputs_.front()) {
}

Status
ISink::Prepare(State& state) {
    if (state_ == State::Ready) {
        return Status::OK();
    }

    if (input_.IsFinished() && state_ != State::Finished) {
        state_ = State::Finished;
        OnFinish();
        return Status::OK();
    }

    input_.SetNeeded();
    if (!input_.HasData()) {
        state_ = State::NeedData;
        return Status::OK();
    }

    chunk_ = input_.Pull(true);
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
