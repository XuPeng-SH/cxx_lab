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
    auto ctx = context->delivery_ctx_;
    auto start = chrono::high_resolution_clock::now();
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
        continue;
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
    auto end = chrono::high_resolution_clock::now();
    cout << std::this_thread::get_id() << ". DoDelivery takes " << chrono::duration<double, std::milli>(end-start).count() << endl;
    return true;
}
