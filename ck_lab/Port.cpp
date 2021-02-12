#include "Port.h"

#include <memory>

namespace MyDB {

void
Connect(OutputPort& output, InputPort& input) {
    assert(!input.state_ && output.state_);
    input.output_port_ = &output;
    output.input_port_ = &input;
    input.state_ = std::make_shared<State>();
    output.state_ = input.state_;
}

};
