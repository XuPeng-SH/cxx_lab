#pragma once

#include "IProcessor.h"
#include "Port.h"
#include <vector>

void
CheckSource(const IProcessor& processor);

class Pipe {
 public:
    using OutputPortPtrs = std::vector<MyDB::OutputPort*>;
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

 private:
    Processors processors_;
    OutputPortPtrs output_ports_;

    size_t max_parallel_streams_ = 0;
};
