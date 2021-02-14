#pragma once

#include "IProcessor.h"
#include "Port.h"

#include <vector>
#include <string>
#include <functional>
#include <memory>

void
CheckSource(const IProcessor& processor);
class Pipe;
using PipePtr = std::shared_ptr<Pipe>;
using Pipes = std::vector<PipePtr>;

class Pipe {
 public:
    using OutputPortPtrs = std::vector<MyDB::OutputPort*>;
    using TransformerClosure = std::function<IProcessorPtr()>;

    Pipe() = default;

    Pipe(Processors processors);
    Pipe(const Pipe& other) = delete;
    Pipe& operator=(const Pipe& other) = delete;
    Pipe(Pipe&& other) = default;
    Pipe& operator=(Pipe&& other) = default;

    void
    AddSource(IProcessorPtr source);
    void
    AddTransform(IProcessorPtr transform);

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

    std::string
    ToString() const;

    static PipePtr
    MergePipes(Pipes& pipes);

 private:
    Processors processors_;
    OutputPortPtrs output_ports_;

    size_t max_parallel_streams_ = 0;
};
