#pragma once

#include <string>
#include <memory>
#include <vector>
#include <rocksdb/status.h>
#include <rocksdb/db.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include "database.h"
#include "doc.h"

namespace db {

// TODO: Size limit and more config
class DBCache {
public:
    DBCache& IncrTid() { std::unique_lock<std::shared_timed_mutex> lock(tidmtx_); ++tid_; };
    uint64_t GetAndIncTid() { std::unique_lock<std::shared_timed_mutex> lock(tidmtx_); return tid_++; };
    uint64_t GetTid() const { std::shared_lock<std::shared_timed_mutex> lock(tidmtx_); return tid_; };
    void SetTid(uint64_t tid) { std::unique_lock<std::shared_timed_mutex> lock(tidmtx_);  tid_ = tid; };

    void UpdateSegMap(uint64_t tid, uint64_t sid) {
        std::unique_lock<std::shared_timed_mutex> lock(sidmtx_);
        segmap_[tid] = sid;
    }

    rocksdb::Status GetSegId(uint64_t tid, uint64_t& sid) const {
        std::shared_lock<std::shared_timed_mutex> lock(sidmtx_);
        const auto& it_map = segmap_.find(tid);
        if (it_map == segmap_.end()) return rocksdb::Status::NotFound();
        sid = it_map->second;
        return rocksdb::Status::OK();
    }

    rocksdb::Status GetAndIncSegId(uint64_t tid, uint64_t& sid) {
        std::unique_lock<std::shared_timed_mutex> lock(sidmtx_);
        const auto& it_map = segmap_.find(tid);
        if (it_map == segmap_.end()) return rocksdb::Status::NotFound();
        sid = it_map->second;
        segmap_[tid]++;
        return rocksdb::Status::OK();
    }

private:
    mutable std::shared_timed_mutex tidmtx_;
    mutable std::shared_timed_mutex sidmtx_;
    std::atomic<uint64_t> tid_;
    std::map<uint64_t, uint64_t> segmap_;
};

class RocksDBImpl : public DBImpl {
public:
    RocksDBImpl(std::shared_ptr<rocksdb::DB> db);
    rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema) override;

    rocksdb::Status GetLatestTableId(uint64_t tid);

protected:
    void Init();

    rocksdb::Status StoreSchema(const DocSchema& schema);

    std::shared_ptr<rocksdb::DB> db_;
    std::shared_ptr<DBCache> db_cache_;
    rocksdb::ReadOptions rdopt_;
    rocksdb::WriteOptions wdopt_;
};

}
