#pragma once

#include <limits>
#include <memory>
#include <vector>

#include "Port.h"

using ProcessorId = uint64_t;
using PortId = uint64_t;
using PortIds = std::vector<PortId>;

constexpr ProcessorId InvalidProcessorId = std::numeric_limits<ProcessorId>::max();

struct IProcessor {
    enum class Status {
        NeedData,
        PortFull,
        Finished,
        Ready
    };

    IProcessor() = default;
    IProcessor(const MyDB::InputPorts& inputs, const MyDB::OutputPorts& outputs) :
        inputs_(inputs), outputs_(outputs) {
        for (auto& port : inputs_) {
            port.processor_ = this;
        }
        for (auto& port : outputs_) {
            port.processor_ = this;
        }
    }

    PortId
    GetOutputPortId(const MyDB::OutputPort* output_port) const {
        PortId id = 0;
        for (const auto& port : outputs_) {
            if (&port == output_port) {
                return id;
            }
            ++id;
        }
        assert(false);
    }
    PortId
    GetInputPortId(const MyDB::InputPort* input_port) const {
        PortId id = 0;
        for (const auto& port : inputs_) {
            if (&port == input_port) {
                return id;
            }
            ++id;
        }
        assert(false);
    }

    auto&
    GetInputs() { return inputs_; }
    auto&
    GetOutputs() { return outputs_; }

    MyDB::InputPorts inputs_;
    MyDB::OutputPorts outputs_;
};

using IProcessorPtr = std::shared_ptr<IProcessor>;
using Processors = std::vector<IProcessor*>;
