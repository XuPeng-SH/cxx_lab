#pragma once

#include "IProcessor.h"
#include "Port.h"

#include <vector>
#include <string>
#include <functional>

void
CheckSource(const IProcessor& processor);

class Pipe {
 public:
    using OutputPortPtrs = std::vector<MyDB::OutputPort*>;
    using TransformerClosure = std::function<IProcessorPtr()>;

    Pipe() = default;

    Pipe(Processors processors);

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

 private:
    Processors processors_;
    OutputPortPtrs output_ports_;

    size_t max_parallel_streams_ = 0;
};
