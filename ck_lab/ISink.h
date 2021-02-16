#pragma once

#include "IProcessor.h"
#include "Chunk.h"

#include <memory>

class ISink : public IProcessor {
 public:
    ISink();

    Status
    Prepare(State&) override;
    Status
    Work() override;

 protected:
    virtual Status
    Consume(Chunk chunk) = 0;

    virtual Status
    OnFinish();

    Chunk chunk_;
    MyDB::InputPort& input_;
};

using ISinkPtr = std::shared_ptr<ISink>;
