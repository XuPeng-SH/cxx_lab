#pragma once

#include <memory>
#include <vector>
#include "Pipe.h"
#include "Status.h"

class Pipeline {
 public:
    Pipeline() = default;
    Pipeline(Pipeline&&) = default;
    Pipeline& operator=(Pipeline&&) = default;
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    void
    Reset();
    Status
    Initialize(PipePtr pipe);

    bool
    IsInitialized() const {
        return !pipe_->Empty();
    }
    bool
    IsCompleted() const {
        return pipe_->IsCompleted();
    }

    Status
    AddTransform(IProcessorPtr transformer);

 private:
    Status
    MakeSureInitilizedAndNotCompleted() const;

    PipePtr pipe_;
};

using PipelinePtr = std::shared_ptr<Pipeline>;
using Pipelines = std::vector<Pipeline>;
