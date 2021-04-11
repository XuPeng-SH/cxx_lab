#pragma once
#include "types.h"
#include "Collector.h"
#include <memory>
#include <string>
#include <sstream>

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
    ID_TYPE w_id = 0;
    ID_TYPE d_id = 0;
    ID_TYPE threshold = 0;
};
using StockLevelContextPtr = std::shared_ptr<StockLevelContext>;

struct OrderStatusContext {
    ID_TYPE c_id = 0;
    ID_TYPE w_id = 0;
    ID_TYPE d_id = 0;
    std::string c_last;
};
using OrderStatusContextPtr = std::shared_ptr<OrderStatusContext>;

struct PaymentContext {
    ID_TYPE w_id = 0;
    ID_TYPE d_id = 0;
    float h_amount = 0;
    ID_TYPE c_w_id = 0;
    ID_TYPE c_d_id = 0;
    ID_TYPE c_id = 0;
    std::string c_last;
    std::string h_date;
};
using PaymentContextPtr = std::shared_ptr<PaymentContext>;

struct NewOrderContext {
    ID_TYPE w_id = 0;
    ID_TYPE d_id = 0;
    ID_TYPE c_id = 0;
    ID_TYPE ol_cnt = 0;
    std::string o_entry_d;
    IDS_TYPE i_ids;
    IDS_TYPE i_w_ids;
    IDS_TYPE i_qtys;
    bool all_local = true;

    std::string
    ToString(const std::string& prefix = "") const {
        std::stringstream ss;
        ss << "[NewOrderContext";
        if (prefix != "") {
            ss << "-" << prefix;
        }
        ss << "] " << "w_id=" << w_id << " d_id=" << d_id << " c_id= " << c_id;
        return ss.str();
    }
};
using NewOrderContextPtr = std::shared_ptr<NewOrderContext>;

struct TpccContext {
    std::string
    TypeStr() const {
        if (type_ == ContextType::DELIVERY) { return "DELIVERY"; }
        else if (type_ == ContextType::NEW_ORDER) { return "NEW_ORDER"; }
        else if (type_ == ContextType::ORDER_STATUS) { return "ORDER_STATUS"; }
        else if (type_ == ContextType::PAYMENT) { return "PAYMENT"; }
        else if (type_ == ContextType::STOCK_LEVEL) { return "STOCK_LEVEL"; }
        else { return "INVALID"; }
    }
    ContextType type_ = ContextType::INVALID;
    DeliveryContextPtr delivery_ctx_ = nullptr;
    StockLevelContextPtr stock_level_ctx_ = nullptr;
    OrderStatusContextPtr order_status_ctx_ = nullptr;
    PaymentContextPtr payment_ctx_ = nullptr;
    NewOrderContextPtr new_order_ctx_ = nullptr;
    Collector& collector_;
    bool has_rollbacked_ = false;
    explicit TpccContext(Collector& collector) : collector_(collector) {}
};
using TpccContextPtr = std::shared_ptr<TpccContext>;
