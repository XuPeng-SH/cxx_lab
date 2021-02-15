#include "Pipeline.h"

void
Pipeline::Reset() {
    pipe_ = nullptr;
}

Status
Pipeline::Initialize(PipePtr pipe) {
    if (IsInitialized()) {
        return Status(PIPELINE_INITIALIZED, "Initialized");
    }

    if (pipe->Empty()) {
        return Status(PIPE_EMPTY, "Empty");
    }

    pipe_ = nullptr;
    pipe_.swap(pipe);

    return Status::OK();
}

Status
Pipeline::MakeSureInitilizedAndNotCompleted() const {
    if (!IsInitialized()) {
        return Status(PIPELINE_NOT_INITIALIZED, "NotInitialized");
    }

    if (IsCompleted()) {
        return Status(PIPELINE_COMPLETED, "AlreadyCompleted");
    }

    return Status::OK();
}

Status
Pipeline::AddTransform(IProcessorPtr transformer) {
    Status status;
    STATUS_CHECK(MakeSureInitilizedAndNotCompleted());
    pipe_->AddTransform(transformer);
    return status;
}
