#pragma once

#include "IProcessor.h"

#include <memory>

class ISink : public IProcessor {
 public:
    ISink() = default;

    Status
    Prepare(State&) override;
    Status
    Work() override;
};

using ISinkPtr = std::shared_ptr<ISinkPtr>;
