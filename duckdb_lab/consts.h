#pragma once
#include "types.h"

//  Item constants
constexpr size_t NUM_ITEMS = 100000;
constexpr int MIN_IM = 1;
constexpr int MAX_IM = 10000;
constexpr float MIN_PRICE = 1.00;
constexpr float MAX_PRICE = 100.00;
constexpr int MIN_I_NAME = 14;
constexpr int MAX_I_NAME = 24;
constexpr int MIN_I_DATA = 26;
constexpr int MAX_I_DATA = 50;

//  Warehouse constants
constexpr int MIN_TAX = 0;
constexpr float MAX_TAX = 0.2000;
constexpr int TAX_DECIMALS = 4;
constexpr float INITIAL_W_YTD = 300000.00;
constexpr int MIN_NAME = 6;
constexpr int MAX_NAME = 10;
constexpr int MIN_STREET = 10;
constexpr int MAX_STREET = 20;
constexpr int MIN_CITY = 10;
constexpr int MAX_CITY = 20;
constexpr int STATE = 2;
constexpr int ZIP_LENGTH = 9;
constexpr const char* ZIP_SUFFIX = "11111";

//  Stock constants
constexpr int MIN_QUANTITY = 10;
constexpr int MAX_QUANTITY = 100;
constexpr int DIST = 24;
constexpr int STOCK_PER_WAREHOUSE = 100000;

//  District constants
constexpr int DISTRICTS_PER_WAREHOUSE = 10;
constexpr float INITIAL_D_YTD = 30000.00;  //  different from Warehouse
constexpr int INITIAL_NEXT_O_ID = 3001;

//  Customer constants
constexpr int CUSTOMERS_PER_DISTRICT = 3000;
constexpr float INITIAL_CREDIT_LIM = 50000.00;
constexpr float MIN_DISCOUNT = 0.0000;
constexpr float MAX_DISCOUNT = 0.5000;
constexpr int DISCOUNT_DECIMALS = 4;
constexpr float INITIAL_BALANCE = -10.00;
constexpr float INITIAL_YTD_PAYMENT = 10.00;
constexpr int INITIAL_PAYMENT_CNT = 1;
constexpr int INITIAL_DELIVERY_CNT = 0;
constexpr int MIN_FIRST = 6;
constexpr int MAX_FIRST = 10;
constexpr const char* MIDDLE = "OE";
constexpr int PHONE = 16;
constexpr int MIN_C_DATA = 300;
constexpr int MAX_C_DATA = 500;
constexpr const char* GOOD_CREDIT = "GC";
constexpr const char* BAD_CREDIT = "BC";

//  Order constants
constexpr int MIN_CARRIER_ID = 1;
constexpr int MAX_CARRIER_ID = 10;
//  HACK: This is not strictly correct, but it works
constexpr int NULL_CARRIER_ID = 0;
//  o_id < than this value, carrier != null, >= -> carrier == null
constexpr int NULL_CARRIER_LOWER_BOUND = 2101;
constexpr int MIN_OL_CNT = 5;
constexpr int MAX_OL_CNT = 15;
constexpr int INITIAL_ALL_LOCAL = 1;
constexpr int INITIAL_ORDERS_PER_DISTRICT = 3000;

//  Used to generate new order transactions
constexpr int MAX_OL_QUANTITY = 10;

//  Order line constants
constexpr int INITIAL_QUANTITY = 5;
constexpr float MIN_AMOUNT = 0.01;

//  History constants
constexpr int MIN_DATA = 12;
constexpr int MAX_DATA = 24;
constexpr float INITIAL_AMOUNT = 10.00;

//  New order constants
constexpr int INITIAL_NEW_ORDERS_PER_DISTRICT = 900;

//  TPC-C 2.4.3.4 (page 31) says this must be displayed when new order rolls back.
constexpr const char* INVALID_ITEM_MESSAGE = "Item number is not valid";

//  Used to generate stock level transactions
constexpr int MIN_STOCK_LEVEL_THRESHOLD = 10;
constexpr int MAX_STOCK_LEVEL_THRESHOLD = 20;

//  Used to generate payment transactions
constexpr float MIN_PAYMENT = 1.0;
constexpr float MAX_PAYMENT = 5000.0;

//  Indicates "brand" items and stock in i_data and s_data.
constexpr const char* ORIGINAL_STRING = "ORIGINAL";

// Table Names
constexpr const char* TABLENAME_ITEM       = "ITEM";
constexpr const char* TABLENAME_WAREHOUSE  = "WAREHOUSE";
constexpr const char* TABLENAME_CUSTOMER   = "CUSTOMER";
constexpr const char* TABLENAME_STOCK      = "STOCK";
constexpr const char* TABLENAME_ORDERS     = "ORDERS";
constexpr const char* TABLENAME_NEW_ORDER  = "NEW_ORDER";
constexpr const char* TABLENAME_ORDER_LINE = "ORDER_LINE";
constexpr const char* TABLENAME_HISTORY    = "HISTORY";
