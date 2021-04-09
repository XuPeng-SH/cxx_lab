#pragma once
#include <string>
#include <sstream>
#include <iomanip>

std::string
DELIVERY_GetNewOrder(ID_TYPE no_d_id, ID_TYPE no_w_id) {
    std::stringstream ss;
    ss << "SELECT NO_O_ID FROM NEW_ORDER WHERE NO_D_ID = " << no_d_id;
    ss << " AND NO_W_ID = " << no_w_id;
    ss << " AND NO_O_ID > -1 LIMIT 1";
    return std::move(ss.str());
}

std::string
DELIVERY_DeleteNewOrder(ID_TYPE no_d_id, ID_TYPE no_w_id, ID_TYPE no_o_id) {
    std::stringstream ss;
    ss << "DELETE FROM NEW_ORDER WHERE NO_D_ID = " << no_d_id;
    ss << " AND NO_W_ID = " << no_w_id << " AND NO_O_ID = " << no_o_id;
    return std::move(ss.str());
}

std::string
DELIVERY_GetCid(ID_TYPE o_id, ID_TYPE o_d_id, ID_TYPE o_w_id) {
    std::stringstream ss;
    ss << "SELECT O_C_ID FROM ORDERS WHERE O_ID = " << o_id << " AND O_D_ID = " << o_d_id;
    ss << " AND O_W_ID = " << o_w_id;
    return std::move(ss.str());
}

std::string
DELIVERY_UpdateOrders(ID_TYPE o_carrier_id, ID_TYPE o_id, ID_TYPE o_d_id, ID_TYPE o_w_id) {
    std::stringstream ss;
    ss << "UPDATE ORDERS SET O_CARRIER_ID = " << o_carrier_id << " WHERE O_ID = " << o_id;
    ss << " AND O_D_ID = " << o_d_id << "  AND O_W_ID = " << o_w_id;
    return std::move(ss.str());
}

std::string
DELIVERY_UpdateOrderLine(const std::string& ol_delivery_date, ID_TYPE ol_o_id, ID_TYPE ol_d_id, ID_TYPE ol_w_id) {
    std::stringstream ss;
    ss << "UPDATE ORDER_LINE SET OL_DELIVERY_D = '" << ol_delivery_date << "' WHERE OL_O_ID = ";
    ss << ol_o_id << " AND OL_D_ID = " << ol_d_id << "  AND OL_W_ID = " << ol_w_id;
    return std::move(ss.str());
}

std::string
DELIVERY_SumOLAmount(ID_TYPE ol_o_id, ID_TYPE ol_d_id, ID_TYPE ol_w_id) {
    std::stringstream ss;
    ss << "SELECT SUM(OL_AMOUNT) FROM ORDER_LINE WHERE OL_O_ID =" << ol_o_id << " AND OL_D_ID = ";
    ss << ol_d_id << " AND OL_W_ID = " << ol_w_id;
    return std::move(ss.str());
}

std::string
DELIVERY_UpdateCustomer(ID_TYPE ol_total, ID_TYPE c_id, ID_TYPE c_d_id, ID_TYPE c_w_id) {
    std::stringstream ss;
    ss << "UPDATE CUSTOMER SET C_BALANCE = C_BALANCE + " << ol_total << "  WHERE C_ID = ";
    ss << c_id << " AND C_D_ID = " << c_d_id << " AND C_W_ID = " << c_w_id;
    return std::move(ss.str());
}

std::string
NEWORDER_GetWarehouseTaxRate(ID_TYPE w_id) {
    std::stringstream ss;
    ss << "SELECT W_TAX FROM WAREHOUSE WHERE W_ID = " << w_id;
    return std::move(ss.str());
}

std::string
NEWORDER_GetDistrict(ID_TYPE o_id, ID_TYPE d_w_id) {
    std::stringstream ss;
    ss << "SELECT D_TAX, D_NEXT_O_ID FROM DISTRICT WHERE D_ID = " << o_id << " AND D_W_ID = " << d_w_id;
    return std::move(ss.str());
}

std::string
NEWORDER_IncrementNextOrderId(ID_TYPE d_next_o_id, ID_TYPE d_id, ID_TYPE d_w_id) {
    std::stringstream ss;
    ss << "UPDATE DISTRICT SET D_NEXT_O_ID = " << d_next_o_id << " WHERE D_ID = ";
    ss << d_id << " AND D_W_ID = " << d_w_id;
    return std::move(ss.str());
}

std::string
NEWORDER_GetCustomer(ID_TYPE c_w_id, ID_TYPE c_d_id, ID_TYPE c_id) {
    std::stringstream ss;
    ss << "SELECT C_DISCOUNT, C_LAST, C_CREDIT FROM CUSTOMER WHERE C_W_ID = " << c_w_id;
    ss << " AND C_D_ID = " << c_d_id << " AND C_ID = " << c_id;
    return std::move(ss.str());
}

std::string
NEWORDER_CreateOrder(ID_TYPE d_next_o_id, ID_TYPE d_id, ID_TYPE w_id, ID_TYPE c_id, const std::string& o_entry_d,
        ID_TYPE o_carrier_id, ID_TYPE o_ol_cnt, ID_TYPE o_all_local) {
    std::stringstream ss;
    ss << "INSERT INTO ORDERS (O_ID, O_D_ID, O_W_ID, O_C_ID, O_ENTRY_D, O_CARRIER_ID, O_OL_CNT, O_ALL_LOCAL) VALUES (";
    ss << d_next_o_id << ", " << d_id << ", " << w_id << ", " << c_id << ", '" << o_entry_d << "', ";
    ss << o_carrier_id << ", " << o_ol_cnt << ", " << o_all_local << ")";
    return std::move(ss.str());
}

std::string
NEWORDER_CreateNewOrder(ID_TYPE o_id, ID_TYPE no_d_id, ID_TYPE no_w_id) {
    std::stringstream ss;
    ss << "INSERT INTO NEW_ORDER (NO_O_ID, NO_D_ID, NO_W_ID) VALUES (";
    ss << o_id << ", " << no_d_id << ", " << no_w_id << ")";
    return std::move(ss.str());
}

std::string
NEWORDER_GetItemInfo(ID_TYPE ol_i_id) {
    std::stringstream ss;
    ss << "SELECT I_PRICE, I_NAME, I_DATA FROM ITEM WHERE I_ID = " << ol_i_id;
    return std::move(ss.str());
}

std::string
NEWORDER_GetStockInfo(ID_TYPE d_id, ID_TYPE ol_i_id, ID_TYPE ol_supply_w_id) {
    std::stringstream ss;
    ss << "SELECT S_QUANTITY, S_DATA, S_YTD, S_ORDER_CNT, S_REMOTE_CNT, S_DIST_";
    ss << std::left << std::setfill('0') << std::setw(2) << d_id;
    ss << " FROM STOCK WHERE   S_I_ID = " << ol_i_id << " AND S_W_ID = " << ol_supply_w_id;
    return std::move(ss.str());
}

std::string
NEWORDER_UpdateStock(ID_TYPE s_quantity, ID_TYPE s_ytd, ID_TYPE s_order_cnt, ID_TYPE s_remote_cnt,
        ID_TYPE ol_i_id, ID_TYPE ol_supply_w_id) {
    std::stringstream ss;
    ss << "UPDATE STOCK SET S_QUANTITY = " << s_quantity << ", S_YTD = " << s_ytd;
    ss << ", S_ORDER_CNT = " << s_order_cnt << ", S_REMOTE_CNT = " << s_remote_cnt;
    ss << " WHERE S_I_ID = " << ol_i_id << " AND S_W_ID = " << ol_supply_w_id;
    return std::move(ss.str());
}

        /* query = NEWORDER_CreateOrderLine(d_next_o_id, ctx->d_id, ctx->w_id, ol_number, ol_i_id, ol_supply_w_id, */
        /*         ctx->o_entry_d, ol_quantity, ol_amount, s_dist_xx); */
std::string
NEWORDER_CreateOrderLine(ID_TYPE o_id, ID_TYPE d_id, ID_TYPE w_id, ID_TYPE ol_number, ID_TYPE ol_i_id,
        ID_TYPE ol_supply_w_id, const std::string& o_entry_d, ID_TYPE ol_quantity, ID_TYPE ol_amount, const std::string& ol_dist_info) {
    std::stringstream ss;
    ss << "INSERT INTO ORDER_LINE (OL_O_ID, OL_D_ID, OL_W_ID, OL_NUMBER, OL_I_ID, OL_SUPPLY_W_ID, ";
    ss << "OL_DELIVERY_D, OL_QUANTITY, OL_AMOUNT, OL_DIST_INFO) VALUES (" << o_id << ", " << d_id << ", ";
    ss << w_id << ", " << ol_number << ", " << ol_i_id << ", " << ol_supply_w_id << ", " << o_entry_d << ", ";
    ss << ol_quantity << ", " << ol_amount << ", '" << ol_dist_info << "')";
    return std::move(ss.str());
}
