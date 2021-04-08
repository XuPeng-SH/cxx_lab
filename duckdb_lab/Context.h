#pragma once
#include "types.h"
#include <string>

struct DeliveryContext {
    ID_TYPE w_id = 0;
    ID_TYPE o_carrier_id = 0;
    std::string ol_delivery_d;
};
