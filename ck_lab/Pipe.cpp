#include "Pipe.h"

#include <assert.h>
#include <unordered_set>

void
CheckSource(const IProcessor& processor) {
    assert(processor.InputPortSize() == 0);
    assert(processor.OutputPortSize() == 1);
}

Pipe::Pipe(Processors processors) : processors_(processors) {
    std::unordered_set<const IProcessor*> set;
    for (const auto& processor : processors_) {
        set.emplace(processor.get());
    }

    for (auto& processor : processors_) {
        for (const auto& port : processor->GetInputs()) {
            assert(port.IsConnected());
            const auto* conneted_processor = &port.ImmutableOutputPort().IMutableProcessor();
            assert(set.count(conneted_processor) != 0);
        }

        for (auto& port : processor->GetOutputs()) {
            if (!port.IsConnected()) {
                output_ports_.push_back(&port);
                continue;
            }
            const auto* conneted_processor = &port.ImmutableInputPort().IMutableProcessor();
            assert(set.count(conneted_processor) != 0);
        }
    }

    assert(!output_ports_.empty());
    max_parallel_streams_ = output_ports_.size();
}

void
Pipe::AddSource(IProcessorPtr processor) {
    CheckSource(*processor);
    assert(!processor->GetOutputs().front().IsConnected());
    output_ports_.push_back(&processor->GetOutputs().front());
    processors_.emplace_back(std::move(processor));
    max_parallel_streams_ = std::max(max_parallel_streams_, output_ports_.size());
}

void
Pipe::AddTransform(IProcessorPtr transform) {
    assert(output_ports_.size() > 0);
    assert(transform->InputPortSize() == OutputPortSize());
    assert(transform->OutputPortSize() > 0);

    auto it_output = output_ports_.begin();
    auto it_input = transform->GetInputs().begin();
    for (; it_output != output_ports_.end(); ++it_output, ++it_input) {
        Connect(**it_output, *it_input);
    }

    output_ports_.clear();
    output_ports_.reserve(transform->OutputPortSize());
    for (auto& port : transform->GetOutputs()) {
        if (port.IsConnected()) {
            continue;
        }

        output_ports_.push_back(&port);
    }

    processors_.emplace_back(std::move(transform));
    max_parallel_streams_ = std::max(max_parallel_streams_, output_ports_.size());
}
