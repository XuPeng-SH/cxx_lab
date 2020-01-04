#include "database.h"
#include <iostream>

using namespace std;

namespace db {

rocksdb::Status MyDB::CreateTable(const std::string& table_name, const DocSchema& schema) {
    return impl_->CreateTable(table_name, schema);
}

}
