#include "Pipe.h"

#include <assert.h>
#include <unordered_set>
#include <sstream>
#include <set>
#include <string>

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

std::string
Pipe::ToString() const {
    std::vector<IProcessor*> sources;
    for (auto& processor : processors_) {
        if (processor->InputPortSize() == 0) {
            sources.emplace_back(processor.get());
        }
    }

    std::set<IProcessor*> printed;
    std::vector<std::vector<std::string>> lines;
    for (auto& source : sources) {
        std::vector<std::string> line;
        IProcessor* current = source;
        while (current) {
            if (printed.find(current) == printed.end()) {
                line.push_back(current->ToString(true));
                printed.insert(current);
                /* std::cout << current->ToString(true) << std::endl; */
            }
            if (current->OutputPortSize() == 0 || !current->GetOutputs().front().IsConnected()) {
                current = nullptr;
            } else {
                /* std::cout << "0x" << (void*)(current) << std::endl; */
                auto& input_port = current->GetOutputs().front().MutableInputPort();
                if (!input_port.IsConnected()) {
                    current = nullptr;
                } else {
                    current = &input_port.MutableProcessor();
                }
            }
        }
        lines.emplace_back(std::move(line));
    }

    std::stringstream ss;
    bool first_line = true;

    auto i = 1;

    for (auto& line : lines) {
        if (!first_line) {
            ss << "\n";
        }

        ss << i++ << ") ";
        bool first = true;
        for (auto& node : line) {
            if (!first) {
                ss << "-->";
            }
            ss << node;
            first = false;
        }
        first_line = false;
    }

    return ss.str();
}

void
RemoveEmptyPipes(Pipes& pipes) {
    for (auto it = pipes.begin(); it != pipes.end(); ++it) {
        if ((*it)->Empty()) {
            it = pipes.erase(it);
        }
    }
}

PipePtr
Pipe::MergePipes(Pipes& pipes) {
    RemoveEmptyPipes(pipes);
    if (pipes.size() == 0) {
        return nullptr;
    }
    if (pipes.size() == 1) {
        return std::make_shared<Pipe>(std::move(*pipes.front()));
    }

    auto pipe = std::make_shared<Pipe>();

    for (auto& p : pipes) {
        pipe->max_parallel_streams_ += p->max_parallel_streams_;
        pipe->output_ports_.insert(pipe->output_ports_.end(), p->output_ports_.begin(), p->output_ports_.end());
        pipe->processors_.insert(pipe->processors_.end(), p->processors_.begin(), p->processors_.end());
    }

    return pipe;
}
