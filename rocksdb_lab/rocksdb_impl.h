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
    void SetTid(uint64_t tid, const std::string* tname = nullptr, const DocSchema* schema = nullptr) {
        std::unique_lock<std::shared_timed_mutex> lock(tidmtx_);
        if (tname != nullptr && schema != nullptr) {
            tnamenamp_[*tname] = tid_;
            tidnamemap_[tid_] = *tname;
            tschemaamp_[*tname] = std::make_shared<DocSchema>(*schema);
        }
        tid_ = tid;
    };

    void UpdateTableMapping(uint64_t tid, const std::string& tname) {
        std::unique_lock<std::shared_timed_mutex> lock(tidmtx_);
        tnamenamp_[tname] = tid;
        tidnamemap_[tid] = tname;
    }

    void UpdateTableMapping(uint64_t tid, const DocSchema& schema) {
        std::unique_lock<std::shared_timed_mutex> lock(tidmtx_);
        tschemaamp_[tidnamemap_[tid]] = std::make_shared<DocSchema>(schema);
    }

    void UpdateSegMap(uint64_t tid, uint64_t sid) {
        std::unique_lock<std::shared_timed_mutex> lock(sidmtx_);
        segmap_[tid] = sid;
    }

    void UpdateTidOffset(uint64_t tid, uint64_t offset) {
        std::unique_lock<std::shared_timed_mutex> lock(sidmtx_);
        offset_[tid] = offset;
    }

    rocksdb::Status GetTidOffset(uint64_t tid, uint64_t& offset) {
        std::shared_lock<std::shared_timed_mutex> lock(sidmtx_);
        /* offset_[tid] = offset; */
        offset = offset_[tid];
        return rocksdb::Status::OK();
    }

    rocksdb::Status GetSegId(uint64_t tid, uint64_t& sid) const {
        std::shared_lock<std::shared_timed_mutex> lock(sidmtx_);
        const auto& it_map = segmap_.find(tid);
        if (it_map == segmap_.end()) return rocksdb::Status::NotFound();
        sid = it_map->second;
        return rocksdb::Status::OK();
    }

    /* rocksdb::Status GetAndIncSegId(uint64_t tid, uint64_t& sid) { */
    /*     std::unique_lock<std::shared_timed_mutex> lock(sidmtx_); */
    /*     const auto& it_map = segmap_.find(tid); */
    /*     if (it_map == segmap_.end()) return rocksdb::Status::NotFound(); */
    /*     sid = it_map->second; */
    /*     segmap_[tid]++; */
    /*     return rocksdb::Status::OK(); */
    /* } */

    std::shared_ptr<DocSchema> GetSchemaByTname(const std::string& tname) {
        std::shared_lock<std::shared_timed_mutex> lock(tidmtx_);
        auto it = tschemaamp_.find(tname);
        if (it == tschemaamp_.end()) {
            return nullptr;
        }
        return it->second;
    }

    rocksdb::Status GetTidByTname(const std::string& tname, uint64_t& tid) {
        std::shared_lock<std::shared_timed_mutex> lock(tidmtx_);
        auto it = tnamenamp_.find(tname);
        if (it == tnamenamp_.end()) {
            return rocksdb::Status::NotFound();
        }
        tid = it->second;
        return rocksdb::Status::OK();
    }

    void Dump() const {}

private:
    mutable std::shared_timed_mutex tidmtx_;
    mutable std::shared_timed_mutex sidmtx_;
    std::atomic<uint64_t> tid_;
    std::map<uint64_t, uint64_t> segmap_;
    std::map<std::string, uint64_t> tnamenamp_;
    std::map<uint64_t, std::string> tidnamemap_;
    std::map<std::string, std::shared_ptr<DocSchema>> tschemaamp_;
    std::map<uint64_t, uint64_t> offset_;
};

class RocksDBImpl : public DBImpl {
public:
    RocksDBImpl(std::shared_ptr<rocksdb::DB> db);
    rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema) override;
    rocksdb::Status AddDoc(const std::string& table_name, const Doc& doc) override;

    rocksdb::Status GetLatestTableId(uint64_t tid);

    void Dump(bool do_print) override;

protected:
    void Init();

    /* rocksdb::Status StoreSchema(const DocSchema& schema); */

    std::shared_ptr<rocksdb::DB> db_;
    std::shared_ptr<DBCache> db_cache_;
    rocksdb::ReadOptions rdopt_;
    rocksdb::WriteOptions wdopt_;
};

}
