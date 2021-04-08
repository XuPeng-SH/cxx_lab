#pragma once

#include <duckdb.hpp>
#include <duckdb/main/connection.hpp>
#include <duckdb/main/database.hpp>
#include <memory>

#include "types.h"
#include "Context.h"

class Driver {
 public:
     explicit Driver(std::shared_ptr<duckdb::Connection> conn) : conn_(conn) {
     }

     bool
     DoDelivery(DeliveryContext& ctx);

 /* private: */
     std::shared_ptr<duckdb::Connection> conn_;
     std::shared_ptr<duckdb::DuckDB> db_;
};

using DriverPtr = std::shared_ptr<Driver>;
