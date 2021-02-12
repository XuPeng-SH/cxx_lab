#include "Graph.h"

Graph::Graph(const Processors& processors) {
    nodes_.reserve(processors.size());
    for (auto node = 0; node < processors.size(); ++node) {
        processor_map_[processors[node]] = node;
        nodes_.emplace_back(std::make_unique<Node>(processors[node], node));
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
