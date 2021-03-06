#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>
#include <functional>

#include "IProcessor.h"

struct Graph {
    using JobT = std::function<void()>;
    using ProcessorMap = std::unordered_map<const IProcessor*, ProcessorId>;

    struct Edge {
        Edge(ProcessorId to, bool is_reversed, uint64_t input_port_number, uint64_t output_port_number) :
            to_(to), is_reversed_(is_reversed), input_port_number_(input_port_number), output_port_number_(output_port_number) {
        }

        ProcessorId to_ = InvalidProcessorId;
        bool is_reversed_;

        PortId input_port_number_;
        PortId output_port_number_;
    };

    using Edges = std::list<Edge>;

    struct State {
        std::exception_ptr exception_;
        JobT job_;
        ProcessorId id_ = 0;
    };

    enum class ExecStatus {
        Idle,
        Preparing,
        Executing,
        Finished
    };

    struct Node {
        IProcessor* processor_ = nullptr;
        ProcessorId processor_id_ = 0;
        IProcessor::State last_processor_status_ = IProcessor::State::NeedData;

        Edges direct_edges_;
        Edges back_edges_;

        JobT job_;
        std::exception_ptr exception_;

        bool
        IsSource() const {
            if (processor_ && (processor_->InputPortSize() == 0)) {
                return true;
            }
            return false;
        }
        bool
        IsSink() const {
            if (processor_ && (processor_->OutputPortSize() == 0)) {
                return true;
            }
            return false;
        }

        std::string
        ToString() const;

        /* PortIds inputs */
        Node(IProcessor* processor, ProcessorId processor_id) :
            processor_(processor), processor_id_(processor_id) {}
    };

    using NodePtr = std::unique_ptr<Node>;
    using Nodes = std::vector<NodePtr>;

    Nodes nodes_;
    ProcessorMap processor_map_;

    explicit Graph(const Processors& processors);

    std::string
    ToString() const;

    bool
    AddEdges(ProcessorId processor_id);

    Edge&
    AddEdge(Edges& edges, Edge edge, const IProcessor* from, const IProcessor* to);
};

using GraphPtr = std::shared_ptr<Graph>;
