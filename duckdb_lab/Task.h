#pragma once

#include "Context.h"
#include "runner.h"
#include <memory>

struct Task {
    explicit Task(const TpccContextPtr& context, RunnerPtr runner)
        : context_(context), runner_(runner) {}

    virtual void
    Run();

    virtual ~Task() {}

    TpccContextPtr context_;
    RunnerPtr runner_;
};

using TaskPtr = std::shared_ptr<Task>;
