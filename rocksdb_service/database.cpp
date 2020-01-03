#include "database.h"
#include <iostream>
#include <rocksdb/db.h>

using namespace std;

namespace db {

rocksdb::Status MyDB::CreateTable(const std::string& table_name, const DocSchema& schema) {
    return impl_->CreateTable(table_name, schema);
}

rocksdb::Status RocksDBImpl::CreateTable(const std::string& table_name, const DocSchema& schema) {
    cout << "RocksDBImpl::CreateTable" << endl;
    return rocksdb::Status::OK();
}

}
