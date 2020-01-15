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
rocksdb::Status MyDB::AddDocs(const std::string& table_name,
        const std::vector<std::shared_ptr<Doc>>& docs) {
    return impl_->AddDocs(table_name, docs);
}

rocksdb::Status MyDB::GetDoc(const std::string& table_name, long uid, std::shared_ptr<Doc> doc) {
    return impl_->GetDoc(table_name, uid, doc);
}

rocksdb::Status MyDB::GetDocs(const std::string& table_name, std::vector<std::shared_ptr<Doc>> docs,
        const FieldsFilter& filter) {
    return impl_->GetDocs(table_name, docs, filter);
}

void MyDB::Dump(bool do_print) {
    return impl_->Dump(do_print);
}

}
