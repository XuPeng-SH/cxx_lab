#pragma once

#include "ISink.h"

/// Read everything and close input
class NullSink : public ISink {
 public:
    NullSink() = default;

    std::string
    GetName() const override {
        return "NullSink";
    }

    Status
    Prepare(State&) override {
        input_.Close();
        state_ = State::Finished;
        return Status::OK();
    }

 protected:
    Status
    Consume(Chunk) override {}
};

/// Read everything but do nothing
class EmptySink : public ISink {
 public:
    EmptySink() = default;

    std::string
    GetName() const override {
        return "EmptySink";
    }

 protected:
    Status
    Consume(Chunk) override {}
};
