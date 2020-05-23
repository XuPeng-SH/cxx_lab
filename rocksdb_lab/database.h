#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <rocksdb/status.h>
#include "doc.h"
#include "datatypes.h"

namespace rocksdb {
    class DB;
}

namespace db {

struct FieldFilter {
    std::string upper_bound;
    std::string lower_bound;
    std::string name;
    size_t number = std::numeric_limits<size_t>::max();
};

using FieldsFilter = std::vector<FieldFilter>;

class DBImpl;
class MyDB {
public:
    MyDB(std::shared_ptr<DBImpl> impl) : impl_(impl) {}
    rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema);
    // TODO: return updated or added
    rocksdb::Status AddDoc(const std::string& table_name, const Doc& doc);
    rocksdb::Status AddDocs(const std::string& table_name,
            const std::vector<std::shared_ptr<Doc>>& docs);
    rocksdb::Status GetDoc(const std::string& table_name, long uid, std::shared_ptr<Doc> doc);
    rocksdb::Status GetDocs(const std::string& table_name, std::vector<std::shared_ptr<Doc>> docs, const FieldsFilter& filter);

    rocksdb::Status GetTables(std::vector<TablePtr>& tables);
    rocksdb::Status LoadField(const std::string& table_name, const std::string& field_name,
            std::vector<uint8_t>& data);
    void Dump(bool do_print);

private:
    std::shared_ptr<DBImpl> impl_;
};

class DBImpl {
public:
    virtual rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema) = 0;
    virtual rocksdb::Status AddDoc(const std::string& table_name, const Doc& doc) = 0;
    virtual rocksdb::Status AddDocs(const std::string& table_name,
            const std::vector<std::shared_ptr<Doc>>& docs) = 0;
    virtual rocksdb::Status GetDoc(const std::string& table_name, long uid, std::shared_ptr<Doc> doc) = 0;
    virtual rocksdb::Status GetDocs(const std::string& table_name,
                                    std::vector<std::shared_ptr<Doc>> docs,
                                    const FieldsFilter& filter
                                    ) = 0;
    virtual rocksdb::Status GetTables(std::vector<TablePtr>& tables) = 0;
    virtual rocksdb::Status LoadField(const std::string& table_name, const std::string& field_name,
            std::vector<uint8_t>& data) = 0;
    virtual void Dump(bool do_print) = 0;

    virtual ~DBImpl() {}
};

}
