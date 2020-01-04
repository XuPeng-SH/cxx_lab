#pragma once

#include <string>
#include <memory>
#include <vector>
#include <rocksdb/status.h>
#include <rocksdb/db.h>
#include <atomic>
#include "database.h"
#include "doc.h"

namespace db {

// TODO: Size limit and more config
class DBCache {
public:
    DBCache& IncrTid() { ++tid_; };
    uint64_t GetAndIncTid() { return tid_++; };
    uint64_t GetTid() const { return tid_; };
    void SetTid(uint64_t tid) { tid_ = tid; };

private:
    std::atomic<uint64_t> tid_;
};

class RocksDBImpl : public DBImpl {
public:
    RocksDBImpl(std::shared_ptr<rocksdb::DB> db);
    rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema) override;

    rocksdb::Status GetLatestTableId(uint64_t tid);

protected:
    void Init();

    std::shared_ptr<rocksdb::DB> db_;
    std::shared_ptr<DBCache> db_cache_;
    rocksdb::ReadOptions rdopt_;
    rocksdb::WriteOptions wdopt_;
};

}
