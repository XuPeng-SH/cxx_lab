#include "database.h"
#include <iostream>

using namespace std;

namespace db {

rocksdb::Status MyDB::CreateTable(const std::string& table_name, const DocSchema& schema) {
    return impl_->CreateTable(table_name, schema);
}

rocksdb::Status MyDB::AddDoc(const std::string& table_name, const Doc& doc) {
    return impl_->AddDoc(table_name, doc);
}

rocksdb::Status MyDB::GetDoc(const std::string& table_name, long uid, std::shared_ptr<Doc> doc) {
    return impl_->GetDoc(table_name, uid, doc);
}

void MyDB::Dump(bool do_print) {
    return impl_->Dump(do_print);
}

}
