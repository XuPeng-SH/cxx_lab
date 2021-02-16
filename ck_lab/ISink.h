#pragma once

#include "IProcessor.h"
#include "Chunk.h"

#include <memory>

class ISink : public IProcessor {
 public:
    ISink() = default;

    virtual Status
    Consume(Chunk chunk) = 0;

    Status
    Prepare(State&) override;
    Status
    Work() override;

 private:
    Status
    OnFinish();

    Chunk chunk_;
};

using ISinkPtr = std::shared_ptr<ISink>;
