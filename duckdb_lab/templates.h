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
    std::ios init(NULL);
    std::stringstream ss;
    init.copyfmt(ss);
    ss << "SELECT S_QUANTITY, S_DATA, S_YTD, S_ORDER_CNT, S_REMOTE_CNT, S_DIST_";
    ss << std::left << std::setfill('0') << std::setw(2) << d_id;
    ss << " FROM STOCK WHERE   S_I_ID = " << ol_i_id << " AND S_W_ID = " << ol_supply_w_id;
    ss.copyfmt(init);
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


std::string
ORDER_STATUS_GetCustomerByCustomerId(ID_TYPE w_id, ID_TYPE d_id, ID_TYPE c_id) {
    std::stringstream ss;
    ss << "SELECT C_ID, C_FIRST, C_MIDDLE, C_LAST, C_BALANCE FROM CUSTOMER WHERE C_W_ID = ";
    ss << w_id << " AND C_D_ID = " << d_id << " AND C_ID = " << c_id;
    return std::move(ss.str());
}

std::string
ORDER_STATUS_GetCustomerByLastName(ID_TYPE w_id, ID_TYPE d_id, const std::string& c_last) {
    std::stringstream ss;
    ss << "SELECT C_ID, C_FIRST, C_MIDDLE, C_LAST, C_BALANCE FROM CUSTOMER WHERE C_W_ID = ";
    ss << w_id << " AND C_D_ID = " << d_id << " AND C_LAST = '" << c_last << "' ORDER BY C_FIRST";
    return std::move(ss.str());
}

std::string
ORDER_STATUS_GetLastOrder(ID_TYPE w_id, ID_TYPE d_id, ID_TYPE c_id) {
    std::stringstream ss;
    ss << "SELECT O_ID, O_CARRIER_ID, O_ENTRY_D FROM ORDERS WHERE O_W_ID = " << w_id;
    ss << " AND O_D_ID = " << d_id << " AND O_C_ID = " << c_id << " ORDER BY O_ID DESC LIMIT 1";
    return std::move(ss.str());
}

std::string
ORDER_STATUS_GetOrderLines(ID_TYPE w_id, ID_TYPE d_id, ID_TYPE o_id) {
    std::stringstream ss;
    ss << "SELECT OL_SUPPLY_W_ID, OL_I_ID, OL_QUANTITY, OL_AMOUNT, OL_DELIVERY_D FROM ORDER_LINE WHERE OL_W_ID = ";
    ss << w_id << " AND OL_D_ID = " << d_id << " AND OL_O_ID = " << o_id;
    return std::move(ss.str());
}

std::string
PAYMENT_GetWarehouse(ID_TYPE w_id) {
    std::stringstream ss;
    ss << "SELECT W_NAME, W_STREET_1, W_STREET_2, W_CITY, W_STATE, W_ZIP FROM WAREHOUSE WHERE W_ID = " << w_id;
    return std::move(ss.str());
}

std::string
PAYMENT_UpdateWarehouseBalance(float h_amount, ID_TYPE w_id) {
    std::stringstream ss;
    ss << "UPDATE WAREHOUSE SET W_YTD = W_YTD + " << h_amount << " WHERE W_ID = " << w_id;
    return std::move(ss.str());
}

std::string
PAYMENT_GetDistrict(ID_TYPE w_id, ID_TYPE d_id) {
    std::stringstream ss;
    ss << "SELECT D_NAME, D_STREET_1, D_STREET_2, D_CITY, D_STATE, D_ZIP FROM DISTRICT WHERE D_W_ID = ";
    ss << w_id << " AND D_ID = " << d_id;
    return std::move(ss.str());
}

std::string
PAYMENT_UpdateDistrictBalance(float h_amount, ID_TYPE d_w_id, ID_TYPE d_id) {
    std::stringstream ss;
    ss << "UPDATE DISTRICT SET D_YTD = D_YTD + " << h_amount << " WHERE D_W_ID  = ";
    ss << d_w_id << " AND D_ID = " << d_id;
    return std::move(ss.str());
}

std::string
PAYMENT_GetCustomerByCustomerId(ID_TYPE w_id, ID_TYPE d_id, ID_TYPE c_id) {
    std::stringstream ss;
    ss << "SELECT C_ID, C_FIRST, C_MIDDLE, C_LAST, C_STREET_1, C_STREET_2, C_CITY, C_STATE, C_ZIP, C_PHONE, C_SINCE, ";
    ss << "C_CREDIT, C_CREDIT_LIM, C_DISCOUNT, C_BALANCE, C_YTD_PAYMENT, C_PAYMENT_CNT, C_DATA FROM CUSTOMER WHERE C_W_ID = ";
    ss << w_id << " AND C_D_ID = " << d_id << " AND C_ID = " << c_id;
    return std::move(ss.str());
}

std::string
PAYMENT_GetCustomerByLastName(ID_TYPE w_id, ID_TYPE d_id, const std::string& c_last) {
    std::stringstream ss;
    ss << "SELECT C_ID, C_FIRST, C_MIDDLE, C_LAST, C_STREET_1, C_STREET_2, C_CITY, C_STATE, C_ZIP, C_PHONE, C_SINCE, C_CREDIT, ";
    ss << "C_CREDIT_LIM, C_DISCOUNT, C_BALANCE, C_YTD_PAYMENT, C_PAYMENT_CNT, C_DATA FROM CUSTOMER WHERE C_W_ID = ";
    ss << w_id << " AND C_D_ID = " << d_id << " AND C_LAST = '" << c_last << "' ORDER BY C_FIRST";
    return std::move(ss.str());
}

std::string
PAYMENT_UpdateBCCustomer(float c_balance, float c_ytd_payment, ID_TYPE c_payment_cnt, const std::string& c_date, ID_TYPE c_w_id,
        ID_TYPE c_d_id, ID_TYPE c_id) {
    std::stringstream ss;
    ss << "UPDATE CUSTOMER SET C_BALANCE = " << c_balance << ", C_YTD_PAYMENT = " << c_ytd_payment;
    ss << ", C_PAYMENT_CNT = " << c_payment_cnt << ", C_DATA = '" << c_date << "' WHERE C_W_ID = ";
    ss << c_w_id << " AND C_D_ID = " << c_d_id << " AND C_ID = " << c_id;
    return std::move(ss.str());
}

std::string
PAYMENT_UpdateGCCustomer(float c_balance, float c_ytd_payment, ID_TYPE c_payment_cnt, ID_TYPE c_w_id,
        ID_TYPE c_d_id, ID_TYPE c_id) {
    std::stringstream ss;
    ss << "UPDATE CUSTOMER SET C_BALANCE = " << c_balance << ", C_YTD_PAYMENT = " << c_ytd_payment;
    ss << ", C_PAYMENT_CNT = " << c_payment_cnt << " WHERE C_W_ID = " << c_w_id << " AND C_D_ID = ";
    ss << c_d_id << " AND C_ID = " << c_id;
    return std::move(ss.str());
}

std::string
PAYMENT_InsertHistory(ID_TYPE c_id, ID_TYPE c_d_id, ID_TYPE c_w_id, ID_TYPE d_id, ID_TYPE w_id,
        const std::string& h_date, float h_amount, const std::string& h_data) {
    std::stringstream ss;
    ss << "INSERT INTO HISTORY VALUES (" << c_id << ", " << c_d_id << ", " << c_w_id << ", " << d_id;
    ss << ", " << w_id << ", '" << h_date << "', " << h_amount << ", '" << h_data << "')";
    return std::move(ss.str());
}

std::string
STOCKLEVEL_GetOid(ID_TYPE w_id, ID_TYPE d_id) {
    std::stringstream ss;
    ss << "SELECT D_NEXT_O_ID FROM DISTRICT WHERE D_W_ID = " << w_id << " AND D_ID = " << d_id;
    return std::move(ss.str());
}

std::string
STOCKLEVEL_GetStockCount(ID_TYPE w_id, ID_TYPE d_id, ID_TYPE o_id, ID_TYPE o_id_min,
        ID_TYPE s_w_id, ID_TYPE threshold) {
    std::stringstream ss;
    ss << "SELECT COUNT(DISTINCT(OL_I_ID)) FROM ORDER_LINE, STOCK ";
    ss << "WHERE OL_W_ID = " << w_id << " AND OL_D_ID = " << d_id;
    ss << " AND OL_O_ID < " << o_id << " AND OL_O_ID >= " << o_id_min;
    ss << " AND S_W_ID = " << s_w_id << " AND S_I_ID = OL_I_ID AND S_QUANTITY < " << threshold;
    return std::move(ss.str());
}
