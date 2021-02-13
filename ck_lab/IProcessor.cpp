#include "IProcessor.h"
#include "Port.h"

bool
IProcessor::AsFromConnect(IProcessor* to) {
    if (!to) return false;
    if (to->InputPortSize() != this->OutputPortSize()) {
        return false;
    }

    auto it_output = outputs_.begin();
    auto it_input = to->GetInputs().begin();
    for (; it_output != outputs_.end(); ++it_output, ++it_input) {
        Connect(*it_output, *it_input);
    }
    return true;
}
