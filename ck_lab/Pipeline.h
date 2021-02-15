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
    Initialize(PipePtr& pipe);

    bool
    IsInitialized() const {
        return pipe_ && !pipe_->Empty();
    }
    bool
    IsCompleted() const {
        return pipe_ && pipe_->IsCompleted();
    }
    size_t
    NumStreams() const {
        if (pipe_) {
            return pipe_->OutputPortSize();
        }
        return 0;
    }

    Status
    AddTransform(IProcessorPtr transformer);

    size_t
    MaxThreads() const {
        return max_threads_;
    }
    void
    SetMaxThreads(size_t threads) {
        max_threads_ = threads;
    }
    size_t
    NumThreads() const;

    PipePtr
    DetachPipe();

 private:
    Status
    MakeSureInitilizedAndNotCompleted() const;

    PipePtr pipe_;
    size_t max_threads_ = 0;
};

using PipelinePtr = std::shared_ptr<Pipeline>;
using Pipelines = std::vector<Pipeline>;
