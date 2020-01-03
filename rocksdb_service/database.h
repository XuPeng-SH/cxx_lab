#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <rocksdb/status.h>
#include "doc.h"

namespace rocksdb {
    class DB;
}

namespace db {

class DBImpl;
class MyDB {
public:
    MyDB(std::shared_ptr<DBImpl> impl) : impl_(impl) {}
    rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema);

private:
    std::shared_ptr<DBImpl> impl_;
};

class DBImpl {
public:
    virtual rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema) = 0;

    virtual ~DBImpl() {}
};


class RocksDBImpl : public DBImpl {
public:
    RocksDBImpl(std::shared_ptr<rocksdb::DB> db) : db_(db) {};
    rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema) override;

protected:
    std::shared_ptr<rocksdb::DB> db_;
};

}
