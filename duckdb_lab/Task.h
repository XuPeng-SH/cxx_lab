#pragma once

#include "Context.h"
#include "runner.h"
#include <memory>

struct Task {
    virtual void
    Run() = 0;
    virtual ~Task() {}
};

using TaskPtr = std::shared_ptr<Task>;

struct DeliveryTask : public Task {
    explicit DeliveryTask(const TpccContextPtr& context, RunnerPtr runner)
        : context_(context), runner_(runner) {}

    void
    Run () override;

    TpccContextPtr context_;
    RunnerPtr runner_;
};
