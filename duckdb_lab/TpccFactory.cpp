#include "TpccFactory.h"
#include <assert.h>
#include <memory>
#include <utility>
#include <iostream>
#include <vector>
#include "consts.h"

const std::vector<std::string> TpccMocker::SYLLABLES = std::vector<std::string>(
        {"BAR", "OUGHT", "ABLE", "PRI", "PRES", "ESE", "ANTI", "CALLY", "ATION", "EING"}
        );

bool
TpccSettings::IsValid() const {
    bool valid = true;
    valid &= (stock_level_p_>= 0);
    valid &= (delivery_p_>= 0);
    valid &= (order_status_p_>= 0);
    valid &= (payment_p_>= 0);
    valid &= (new_order_p_>= 0);
    valid &= (stock_level_p_ + delivery_p_ + order_status_p_ + payment_p_ + new_order_p_) == 100;
    if (valid) {
        sl_p_upper_ = stock_level_p_;
        d_p_upper_ = sl_p_upper_ + delivery_p_;
        os_p_upper_ = d_p_upper_ + order_status_p_;
        p_p_upper_ = os_p_upper_ + payment_p_;
    }
    return valid;
}

TpccSettings::TpccSettings(ScaleParametersPtr sp) : sp_(sp) {
}

ScaleParametersPtr
ScaleParameters::Build(size_t scale_factor) {
    assert(scale_factor <= 10000 & scale_factor > 0);
    auto sp = std::make_shared<ScaleParameters>();
    sp->items_ = NUM_ITEMS;
    sp->districts_per_wh_ = DISTRICTS_PER_WAREHOUSE;
    sp->new_orders_per_district_ = INITIAL_NEW_ORDERS_PER_DISTRICT;
    sp->customers_per_district_ = CUSTOMERS_PER_DISTRICT;
    sp->warehouses_ = scale_factor;
    sp->wh_start_ = 1;
    sp->wh_end_ = sp->wh_start_ + sp->warehouses_ - 1;

    return std::move(sp);
}

TpccContextPtr
TpccFactory::NextContext() {
    auto ctx = std::make_shared<TpccContext>(metric_collector_);
    auto rand_val = RandomNumber<int>(1, 100);

    if (rand_val <= settings_->GetStockLevelUpper()) {
        ctx->type_ = ContextType::STOCK_LEVEL;
        auto context = std::make_shared<StockLevelContext>();
        context->w_id = mocker_->MockWarehouseID();
        context->d_id = mocker_->MockDistrictID();
        context->threshold = RandomNumber<int>(MIN_STOCK_LEVEL_THRESHOLD, MAX_STOCK_LEVEL_THRESHOLD);
        ctx->stock_level_ctx_ = context;
    } else if (rand_val <= settings_->GetDeliveryUpper()) {
        ctx->type_ = ContextType::DELIVERY;
        ctx->delivery_ctx_ = std::make_shared<DeliveryContext>();
        ctx->delivery_ctx_->o_carrier_id = RandomNumber<int>(MIN_CARRIER_ID, MAX_CARRIER_ID);
        ctx->delivery_ctx_->w_id = mocker_->MockWarehouseID();
        ctx->delivery_ctx_->ol_delivery_d = CurrentDateTimeString();
    } else if (rand_val <= settings_->GetOrderStatusUpper()) {
        ctx->type_ = ContextType::ORDER_STATUS;
        auto context = std::make_shared<OrderStatusContext>();
        context->w_id = mocker_->MockWarehouseID();
        context->d_id = mocker_->MockDistrictID();
        if (RandomNumber<int>(1, 60) <= 60) {
            context->c_last = mocker_->MockLastName();
        } else {
            context->c_id = mocker_->MockCustomerID();
        }
        ctx->order_status_ctx_ = context;
    } else if (rand_val <= settings_->GetPaymentUpper()) {
        ctx->type_ = ContextType::PAYMENT;
        auto context = std::make_shared<PaymentContext>();
        context->w_id = mocker_->MockWarehouseID();
        context->d_id = mocker_->MockDistrictID();
        context->h_amount = mocker_->MockPaymentAmount(2);
        context->h_date = CurrentDateTimeString();

        if ((settings_->sp_->warehouses_ == 1) || (RandomNumber<int>(1, 100) <= 85)) {
            context->c_w_id = context->w_id;
            context->c_d_id = context->d_id;
        } else {
            // 15%: paying through another warehouse:
            while (true) {
                auto c_w_id = RandomNumber<int>(settings_->sp_->wh_start_, settings_->sp_->wh_start_);
                if (c_w_id != context->w_id) {
                    context->c_w_id = c_w_id;
                    break;
                }
            }
            context->c_d_id = mocker_->MockDistrictID();
        }

        if (RandomNumber<int>(1, 100) <= 60) {
            // 60%: payment by last name
            context->c_last = mocker_->MockLastName();
        } else {
            // 40%: payment by id
            context->c_id = mocker_->MockCustomerID();
        }
        ctx->payment_ctx_ = context;
    } else {
        ctx->type_ = ContextType::NEW_ORDER;
        ctx->new_order_ctx_ = std::make_shared<NewOrderContext>();
        ctx->new_order_ctx_->w_id = mocker_->MockWarehouseID();
        ctx->new_order_ctx_->d_id = mocker_->MockDistrictID();
        ctx->new_order_ctx_->c_id = mocker_->MockCustomerID();
        ctx->new_order_ctx_->ol_cnt = RandomNumber<int>(MIN_OL_CNT, MAX_OL_CNT);
        ctx->new_order_ctx_->o_entry_d = CurrentDateTimeString();

        ctx->new_order_ctx_->i_ids = mocker_->MockItemIDS(ctx->new_order_ctx_->ol_cnt);
        for (auto i = 0; i < ctx->new_order_ctx_->ol_cnt; ++i) {
            int r = RandomNumber<int>(1, 100);
            if ((r == 1) && (settings_->sp_->warehouses_ > 1)) {
                ID_TYPE w_id;
                while (true) {
                    w_id = mocker_->MockWarehouseID();
                    if (w_id != ctx->new_order_ctx_->w_id) {
                        break;
                    }
                }
                ctx->new_order_ctx_->i_w_ids.push_back(w_id);
                ctx->new_order_ctx_->all_local = false;
            } else {
                ctx->new_order_ctx_->i_w_ids.push_back(ctx->new_order_ctx_->w_id);
            }
            ctx->new_order_ctx_->i_qtys.push_back(RandomNumber<ID_TYPE>(1, MAX_OL_QUANTITY));
        }
    }
    return ctx;
}
