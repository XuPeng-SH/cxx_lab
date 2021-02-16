#pragma once

#include "IProcessor.h"
#include "NullSink.h"
#include "Port.h"
#include "Status.h"

#include <vector>
#include <string>
#include <functional>
#include <memory>

Status
CheckSource(const IProcessor& processor);
class Pipe;
using PipePtr = std::unique_ptr<Pipe>;
using Pipes = std::vector<PipePtr>;

class Pipe {
 public:
    using OutputPortPtrs = std::vector<MyDB::OutputPort*>;
    using ProcessorGetter = std::function<IProcessorPtr()>;

    Pipe() = default;

    Pipe(Processors processors);
    Pipe(const Pipe& other) = delete;
    Pipe& operator=(const Pipe& other) = delete;
    Pipe(Pipe&& other) = default;
    Pipe& operator=(Pipe&& other) = default;

    Status
    AddSource(IProcessorPtr source);
    Status
    AddTransform(IProcessorPtr transform);
    Status
    AddSink(const ProcessorGetter&);

    bool
    Empty() const {
        return processors_.empty();
    }

    size_t
    OutputPortSize() const {
        return output_ports_.size();
    }
    size_t
    MaxParallelStreams() const {
        return max_parallel_streams_;
    }
    size_t
    NumOfProcessors() const {
        return processors_.size();
    }

    bool
    IsCompleted() const {
        return !Empty() && OutputPortSize() == 0;
    }

    std::string
    ToString() const;

    static PipePtr
    MergePipes(Pipes& pipes);

 private:
    Processors processors_;
    OutputPortPtrs output_ports_;

    size_t max_parallel_streams_ = 0;
};
