#pragma once
#include "types.h"
#include <memory>
#include <string>

enum class ContextType : uint8_t {
    INVALID = 0,
    STOCK_LEVEL = 1,
    DELIVERY = 2,
    ORDER_STATUS = 3,
    PAYMENT = 4,
    NEW_ORDER = 5
};

struct DeliveryContext {
    ID_TYPE w_id = 0;
    ID_TYPE o_carrier_id = 0;
    std::string ol_delivery_d;
};
using DeliveryContextPtr = std::shared_ptr<DeliveryContext>;

struct StockLevelContext {
};
using StockLevelContextPtr = std::shared_ptr<StockLevelContext>;

struct OrderStatusContext {
};
using OrderStatusContextPtr = std::shared_ptr<OrderStatusContext>;

struct PaymentContext {
};
using PaymentContextPtr = std::shared_ptr<PaymentContext>;

struct NewOrderContext {
};
using NewOrderContextPtr = std::shared_ptr<NewOrderContext>;

struct TpccContext {
    ContextType type_ = ContextType::INVALID;
    DeliveryContextPtr delivery_ctx_ = nullptr;
    StockLevelContextPtr stock_level_ctx_ = nullptr;
    OrderStatusContextPtr order_status_ctx_ = nullptr;
    PaymentContextPtr payment_ctx_ = nullptr;
    NewOrderContextPtr new_order_ctx_ = nullptr;
};
using TpccContextPtr = std::shared_ptr<TpccContext>;
