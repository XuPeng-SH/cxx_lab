#include "Graph.h"

#include <iostream>
#include <sstream>

std::string
Graph::Node::ToString() const {
    std::stringstream ss;
    std::string type = "Node";
    if (IsSource() && IsSink()) {
        type = "Only_Node";
    } else if (IsSource()) {
        type = "Src_Node";
    } else if (IsSink()) {
        type = "Snk_Node";
    }
    ss << "[" << type << "(" << processor_->InputPortSize();
    ss << "," << processor_->OutputPortSize() << ")]";
    return ss.str();
}

Graph::Graph(const Processors& processors) {
    nodes_.reserve(processors.size());
    for (auto node = 0; node < processors.size(); ++node) {
        processor_map_[processors[node]] = node;
        nodes_.emplace_back(std::make_unique<Node>(processors[node], node));
    }

    for (auto node = 0; node < processors.size(); ++ node) {
        AddEdges(node);
    }
}

bool
Graph::AddEdges(ProcessorId processor_id) {
    IProcessor* from = nodes_[processor_id]->processor_;
    bool edge_added = false;

    auto& inputs = from->GetInputs();
    auto pre_input_size = nodes_[processor_id]->back_edges_.size();

    if (pre_input_size < inputs.size()) {
        edge_added = true;
        for (auto it = std::next(inputs.begin(), pre_input_size); it != inputs.end(); ++it, ++pre_input_size) {
            auto& port = it->ImmutableOutputPort();
            /* std::cout << "port addr 0x" << (void*)(&port) << std::endl; */
            auto to_processor = &port.IMutableProcessor();
            auto output_port_id = to_processor->GetOutputPortId(&port);
            Edge edge(0, true, pre_input_size, output_port_id);
            auto& added = AddEdge(nodes_[processor_id]->back_edges_, std::move(edge), from, to_processor);
        }
    }

    auto& outputs = from->GetOutputs();
    auto pre_output_size = nodes_[processor_id]->direct_edges_.size();

    if (pre_output_size < outputs.size()) {
        edge_added = true;
        for (auto it = std::next(outputs.begin(), pre_output_size); it != outputs.end(); ++it, ++pre_output_size) {
            auto& port = it->ImmutableInputPort();
            /* std::cout << "port addr 0x" << (void*)(&port) << std::endl; */
            auto to_processor = &port.IMutableProcessor();
            auto input_port_id = to_processor->GetInputPortId(&port);
            Edge edge(0, false, input_port_id, pre_output_size);
            auto& added = AddEdge(nodes_[processor_id]->direct_edges_, std::move(edge), from, to_processor);
        }
    }
    return edge_added;
}

Graph::Edge&
Graph::AddEdge(Edges& edges, Edge edge, const IProcessor* from, const IProcessor* to) {
    auto it = processor_map_.find(to);
    if (it == processor_map_.end()) {
        assert(false);
    }
    edge.to_ = it->second;
    auto& added = edges.emplace_back(std::move(edge));
    return added;
}

std::string
Graph::ToString() const {
    std::stringstream ss;
    bool first = true;
    for (auto i = 0; i < nodes_.size(); ++i) {
        if (!first) {
            ss << "-->";
        }
        ss << nodes_[i]->ToString();
        first = false;
    }
    return ss.str();
}
