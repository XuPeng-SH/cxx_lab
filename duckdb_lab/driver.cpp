#include "driver.h"
#include <atomic>
#include <memory>

#include <duckdb/common/types/vector.hpp>
#include <thread>
#include <sstream>
#include <iostream>
#include <chrono>

#include "consts.h"
#include "templates.h"

#define CHECK_ROLLBACK(RESULT) \
    if (!RESULT->success) {    \
        cout << std::this_thread::get_id() << ". Query: " << query << " has error: " << RESULT->error << endl; \
        this->conn_->Query("ROLLBACK"); \
        return false;   \
    }

using namespace std;
using namespace duckdb;

bool
Driver::DoDelivery(TpccContextPtr& context) {
    /* return true; */
    assert(context->type_ == ContextType::DELIVERY && context->delivery_ctx_);
    auto ctx = context->delivery_ctx_;
    this->conn_->Query("START TRANSACTION");

    for(ID_TYPE d_id = 1; d_id < DISTRICTS_PER_WAREHOUSE + 1; ++d_id) {
        auto query = DELIVERY_GetNewOrder(d_id, ctx->w_id);
        auto r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        if (r1->collection.Count() == 0) {
            cout << "Query: " << query << " empty" << endl;
            continue;
        }

        auto no_o_id_val = r1->GetValue(0, 0);

        query = DELIVERY_GetCid(no_o_id_val.GetValue<ID_TYPE>(), d_id, ctx->w_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);

        auto c_id_val = r1->GetValue(0, 0);

        query = DELIVERY_SumOLAmount(no_o_id_val.GetValue<ID_TYPE>(), d_id, ctx->w_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        auto ol_total_val = r1->GetValue(0, 0);
        query = DELIVERY_DeleteNewOrder(d_id, ctx->w_id, no_o_id_val.GetValue<ID_TYPE>());
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        query = DELIVERY_UpdateOrders(ctx->o_carrier_id, no_o_id_val.GetValue<ID_TYPE>(), d_id, ctx->w_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        query = DELIVERY_UpdateOrderLine(ctx->ol_delivery_d, no_o_id_val.GetValue<ID_TYPE>(), d_id, ctx->w_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);

        query = DELIVERY_UpdateCustomer(ol_total_val.GetValue<ID_TYPE>(), c_id_val.GetValue<ID_TYPE>(), d_id, ctx->w_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
    }
    this->conn_->Query("COMMIT");
    return true;
}

bool
Driver::DoNewOrder(TpccContextPtr& context) {
    /* return true; */
    assert(context->type_ == ContextType::NEW_ORDER && context->new_order_ctx_);
    auto ctx = context->new_order_ctx_;
    std::string query = "START TRANSACTION";
    auto r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    std::vector<std::vector<Value>> items;
    for (auto& i_id : ctx->i_ids) {
        query = NEWORDER_GetItemInfo(i_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        items.push_back(r1->collection.GetRow(0));
    }
    // ----------------
    // Collect Information from WAREHOUSE, DISTRICT, and CUSTOMER
    // ----------------
    query = NEWORDER_GetWarehouseTaxRate(ctx->w_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);
    /* auto w_tax_val = r1->GetValue(0, 0); */

    query = NEWORDER_GetDistrict(ctx->d_id, ctx->w_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);
    /* auto d_tax_val = r1->GetValue(0, 0); */
    auto d_next_o_id = r1->collection.GetValue(1, 0).GetValue<ID_TYPE>();

    query = NEWORDER_GetCustomer(ctx->w_id, ctx->d_id, ctx->c_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);
    /* auto c_discount_val = r1->GetValue(0, 0); */

    // ----------------
    // Insert Order Information
    // ----------------
    query = NEWORDER_IncrementNextOrderId(d_next_o_id + 1, ctx->d_id, ctx->w_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    query = NEWORDER_CreateOrder(d_next_o_id, ctx->d_id, ctx->w_id, ctx->c_id,ctx->o_entry_d,
            NULL_CARRIER_ID, ctx->i_ids.size(), ctx->all_local);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    query = NEWORDER_CreateNewOrder(d_next_o_id, ctx->d_id, ctx->w_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    // ----------------
    // Insert Order Item Information
    // ----------------

    float total = 0;
    for (auto i = 0; i < ctx->i_ids.size(); ++i) {
        auto ol_number = ctx->i_ids[i] + 1;
        auto ol_supply_w_id = ctx->i_w_ids[i];
        auto ol_i_id = ctx->i_ids[i];
        auto ol_quantity = ctx->i_qtys[i];

        auto& iteminfo = items[i];
        auto i_name = iteminfo[1].GetValue<std::string>();
        auto i_data = iteminfo[2].GetValue<std::string>();
        auto i_price = iteminfo[0].GetValue<float>();

        query = NEWORDER_GetStockInfo(ctx->d_id, ol_i_id, ol_supply_w_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        auto stock_info = r1->collection.GetRow(0);
        if (stock_info.size() == 0) {
            std::cout << "No STOCK record for (ol_i_id=" << ol_i_id << "ol_supply_w_id=";
            std::cout << ol_supply_w_id << ")" << std::endl;
            continue;
        }

        auto s_quantity = stock_info[0].GetValue<ID_TYPE>();
        auto s_ytd = stock_info[2].GetValue<ID_TYPE>();
        auto s_order_cnt = stock_info[3].GetValue<ID_TYPE>();
        auto s_remote_cnt = stock_info[4].GetValue<ID_TYPE>();
        auto s_data = stock_info[1].GetValue<std::string>();
        auto s_dist_xx = stock_info[5].GetValue<std::string>();

        // Update stock
        s_ytd += ol_quantity;
        if (s_quantity >= ol_quantity + 10) {
            s_quantity = s_quantity - ol_quantity;
        } else {
            s_quantity = s_quantity + 91 - ol_quantity;
        }
        s_order_cnt += 1;

        if (ol_supply_w_id != ctx->w_id) {
            s_remote_cnt += 1;
        }

        query = NEWORDER_UpdateStock(s_quantity, s_ytd, s_order_cnt, s_remote_cnt, ol_i_id, ol_supply_w_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);

        std::string brand_generic = "G";
        if ((i_data.find(ORIGINAL_STRING) != std::string::npos)
                && (s_data.find(ORIGINAL_STRING) != std::string::npos)) {
            brand_generic = "B";
        }

        // Transaction profile states to use "ol_quantity * i_price"
        auto ol_amount = ol_quantity * i_price;
        total += ol_amount;

        query = NEWORDER_CreateOrderLine(d_next_o_id, ctx->d_id, ctx->w_id, ol_number, ol_i_id, ol_supply_w_id,
                ctx->o_entry_d, ol_quantity, ol_amount, s_dist_xx);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
    }

    /* this->conn_->Query("ROLLBACK"); */
    this->conn_->Query("COMMIT");
    return true;
}

bool
Driver::DoPayment(TpccContextPtr& context) {
    return true;
    assert(context->type_ == ContextType::PAYMENT && context->payment_ctx_);
    auto ctx = context->payment_ctx_;
    std::string query = "START TRANSACTION";
    auto r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    vector<Value> customer;
    if (ctx->c_last.empty()) {
        query = PAYMENT_GetCustomerByCustomerId(ctx->w_id, ctx->d_id, ctx->c_id);
        /* auto start = chrono::high_resolution_clock::now(); */
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        customer = r1->collection.GetRow(0);
        /* auto end = chrono::high_resolution_clock::now(); */
        /* std::cout << query << std::endl; */
        /* std::cout << "THEREAD[" << std::this_thread::get_id() << "] YYYYYY(" << context->TypeStr() << ") TAKES "; */
        /* std::cout << chrono::duration<double, std::milli>(end-start).count() << std::endl; */
    } else {
        query = PAYMENT_GetCustomerByLastName(ctx->w_id, ctx->d_id, ctx->c_last);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        auto index = int((r1->collection.Count() - 1) / 2);
        customer = r1->collection.GetRow(index);
        ctx->c_id = customer[0].GetValue<ID_TYPE>();
    }
    this->conn_->Query("COMMIT");
    return true;

    auto c_balance = customer[14].GetValue<float>() - ctx->h_amount;
    auto c_ytd_payment = customer[15].GetValue<float>() + ctx->h_amount;
    auto c_payment_cnt = customer[16].GetValue<int>() + 1;
    auto c_data = customer[17].GetValue<std::string>();

    query = PAYMENT_GetWarehouse(ctx->w_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);
    auto warehouse = r1->collection.GetRow(0);

    query = PAYMENT_GetDistrict(ctx->w_id, ctx->d_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);
    auto district = r1->collection.GetRow(0);

    query = PAYMENT_UpdateWarehouseBalance(ctx->h_amount, ctx->w_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    query = PAYMENT_UpdateDistrictBalance(ctx->h_amount, ctx->w_id, ctx->d_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    // Customer Credit Information
    if (customer[11].GetValue<std::string>() == BAD_CREDIT) {
        std::stringstream ss;
        ss << ctx->c_id << " " << ctx->c_d_id << " " << ctx->d_id << " " << ctx->w_id << " " << ctx->h_amount;
        c_data = std::string(ss.str()) + "|" + c_data;
        if (c_data.size() > MAX_C_DATA) {
            c_data = c_data.substr(0, MAX_C_DATA);
        }

        query = PAYMENT_UpdateBCCustomer(c_balance, c_ytd_payment, c_payment_cnt, c_data, ctx->c_w_id,
                ctx->c_d_id, ctx->c_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
    } else {
        query = PAYMENT_UpdateGCCustomer(c_balance, c_ytd_payment, c_payment_cnt, ctx->c_w_id, ctx->c_d_id, ctx->c_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
    }

    // Concatenate w_name, four spaces, d_name
    auto h_data = warehouse[0].GetValue<std::string>() + "    " + district[0].GetValue<std::string>();
    // Create the history record */
    query = PAYMENT_InsertHistory(ctx->c_id, ctx->c_d_id, ctx->c_w_id, ctx->d_id, ctx->w_id, ctx->h_date,
            ctx->h_amount, h_data);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    this->conn_->Query("COMMIT");
    return true;
}
bool
Driver::DoOrderStatus(TpccContextPtr& context) {
    /* return true; */
    assert(context->type_ == ContextType::ORDER_STATUS && context->order_status_ctx_);
    auto ctx = context->order_status_ctx_;
    std::string query = "START TRANSACTION";
    auto r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    if (ctx->c_last.empty()) {
        query = ORDER_STATUS_GetCustomerByCustomerId(ctx->w_id, ctx->d_id, ctx->c_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
    } else {
        // Get the midpoint customer's id
        query = ORDER_STATUS_GetCustomerByLastName(ctx->w_id, ctx->d_id, ctx->c_last);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
        auto index = int((r1->collection.Count() - 1) / 2);
        ctx->c_id = r1->collection.GetValue(0, index).GetValue<ID_TYPE>();
    }

    query = ORDER_STATUS_GetLastOrder(ctx->w_id, ctx->d_id, ctx->c_id);
    r1 = this->conn_->Query(query);
    CHECK_ROLLBACK(r1);

    if (r1->collection.Count() > 0) {
        auto o_id = r1->collection.GetValue(0, 0).GetValue<ID_TYPE>();
        query = ORDER_STATUS_GetOrderLines(ctx->w_id, ctx->d_id, o_id);
        r1 = this->conn_->Query(query);
        CHECK_ROLLBACK(r1);
    }

    this->conn_->Query("COMMIT");
    return true;
}

bool
Driver::DoStockLevel(TpccContextPtr& ctx) { return true; }

void
Driver::ForceRollBack() {
    this->conn_->Query("ROLLBACK");
}
