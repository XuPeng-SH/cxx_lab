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
#include "rocksdb_util.h"

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
        /* std::cout << "updating tname=" << tname << " for tid=" << tid << std::endl; */
        tnamenamp_[tname] = tid;
        tidnamemap_[tid] = tname;
    }

    void UpdateTableMapping(uint64_t tid, const DocSchema& schema) {
        std::unique_lock<std::shared_timed_mutex> lock(tidmtx_);
        /* std::cout << "updating schema for tid=" << tid << std::endl; */
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

    std::shared_ptr<DocSchema> GetSchema(const uint64_t tid) {
       auto& tname = tidnamemap_[tid];
       return GetSchema(tname);
    }

    std::shared_ptr<DocSchema> GetSchema(const std::string& tname) {
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

class KeyHelper {
public:
    static void PrintDBTableNameKey(const rocksdb::Slice& key,
                                    const rocksdb::Slice& val,
                                    std::shared_ptr<DBCache> cache,
                                    const std::string& header = "") {
        uint64_t tid;

        Serializer::Deserialize(val, tid);

        std::cout << header << "[ " << key.ToString() << ", " << tid << " ]" << std::endl;

    }

    static void PrintDBSequenceKey(const rocksdb::Slice& key,
                                   const rocksdb::Slice& val,
                                   std::shared_ptr<DBCache> cache,
                                   const std::string& header = "") {
        uint64_t tid;
        Serializer::Deserialize(val, tid);

        std::cout << header << "[ " << DBTableSequenceKey << ", " << tid << " ]" << std::endl;
    }

    static void PrintDBTableMappingKey(const rocksdb::Slice& key,
                                       const rocksdb::Slice& val,
                                       std::shared_ptr<DBCache> cache,
                                       const std::string& header = "") {
        uint64_t tid;
        auto tid_addr = key.data() + PrefixSize;
        rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
        Serializer::Deserialize(tid_slice, tid);

        DocSchema schema;
        auto s = DocSchema::Deserialize(val, schema);
        if (!s.ok()) {
            std::cout << s.ToString() << std::endl;
        }
        std::cout << header << "[ " << DBTableMappingPrefix << ":" << tid << " ]" << std::endl;
    }

    static void PrintDBSegmentNextIDKey(const rocksdb::Slice& key,
                                        const rocksdb::Slice& val,
                                        std::shared_ptr<DBCache> cache,
                                        const std::string& header = "") {
        uint64_t tid, sid, id;
        auto tid_addr = key.data() + PrefixSize;
        auto sid_addr = val.data();
        auto id_addr = val.data() + sizeof(uint64_t);
        rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
        rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
        rocksdb::Slice id_slice(id_addr, sizeof(id));

        Serializer::Deserialize(tid_slice, tid);
        Serializer::Deserialize(sid_slice, sid);
        Serializer::Deserialize(id_slice, id);

        std::cout << header << "[ " << DBTableSegmentNextIDPrefix << tid << ", " << sid;
        std::cout << ":" << id << " ]" << std::endl;
    }

    static void PrintDBUidIdMappingKey(const rocksdb::Slice& key,
                                       const rocksdb::Slice& val,
                                       std::shared_ptr<DBCache> cache,
                                       const std::string& header = "") {
        uint64_t tid, sid, id;
        long uid;
        auto tid_addr = key.data() + PrefixSize;
        auto uid_addr = tid_addr + sizeof(tid);
        auto sid_addr = val.data();
        auto id_addr = sid_addr + sizeof(sid);

        rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
        rocksdb::Slice uid_slice(uid_addr, sizeof(uid));
        rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
        rocksdb::Slice id_slice(id_addr, sizeof(id));

        Serializer::Deserialize(tid_slice, tid);
        Serializer::Deserialize(sid_slice, sid);
        Serializer::Deserialize(uid_slice, uid);
        Serializer::Deserialize(id_slice, id);

        std::cout << header << "[ " << DBTableUidIdMappingPrefix << ":" << tid << ":" << uid;
        std::cout << ", " << sid << ":" << id << " ]" << std::endl;
    }

    static void PrintDBCurrentSegmentKey(const rocksdb::Slice& key,
                                         const rocksdb::Slice& val,
                                         std::shared_ptr<DBCache> cache,
                                         const std::string& header = "") {

        auto tid_addr = key.data() + PrefixSize;
        uint64_t tid;
        rocksdb::Slice tid_sli(tid_addr, sizeof(tid));
        Serializer::Deserialize(tid_sli, tid);

        uint64_t sid;
        Serializer::Deserialize(val, sid);

        std::cout << header << "[ " << DBTableCurrentSegmentPrefix << ":" << tid << ", " << sid << " ]" << std::endl;
    }

    // [Key]$Prefix:$tid:$sid$id$fid    [val]$fval
    static void PrintDBFieldValueKeyValue(const rocksdb::Slice& key,
                                     const rocksdb::Slice& val,
                                     std::shared_ptr<DBCache> cache,
                                     const std::string& header = "") {
        uint64_t tid, sid, offset;
        uint8_t fid;
        auto tid_addr = key.data() + PrefixSize;
        auto sid_addr = key.data() + PrefixSize + sizeof(tid);
        auto offset_addr = key.data() + PrefixSize + 2*sizeof(uint64_t);
        auto fid_addr = (char*)(tid_addr) + 3*sizeof(uint64_t);

        rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
        rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
        rocksdb::Slice offset_slice(offset_addr, sizeof(offset));
        rocksdb::Slice fid_slice(fid_addr, sizeof(fid));

        Serializer::Deserialize(tid_slice, tid);
        Serializer::Deserialize(sid_slice, sid);
        Serializer::Deserialize(offset_slice, offset);
        Serializer::Deserialize(fid_slice, fid);

        std::cout << header << "[ " << DBTableFieldValuePrefix << ":" << tid << ":" << sid << ":" << offset << ":" << (int)fid;

        auto schema = cache->GetSchema(tid);
        uint8_t field_type;
        auto s = schema->GetFieldType(fid, field_type);
        if (!s) {
            std::cerr << "Cannot get field type of field_id" << (int)fid << std::endl;
            return;
        }

        if (field_type == LongField::FieldTypeValue()) {
            long vv;
            Serializer::Deserialize(val, vv);
            std::cout << "[:]" << vv;
        } else if (field_type == FloatField::FieldTypeValue()) {
            long vv;
            Serializer::Deserialize(val, vv);
            std::cout << "<:>" << vv;
        } else if (field_type == StringField::FieldTypeValue()) {
            std::cout << "{:}" << val.ToString();
        } else if (field_type == FloatVectorField::FieldTypeValue()) {
            std::cout << "(:)< ";
            if (val.size() > 16) {
                std::cout << "...";
            } else {
                float* vec = (float*)val.data();
                for (auto i=0; i<val.size()/sizeof(float); ++i) {
                    std::cout << *(vec+i) << " ";
                }
            }
            std::cout << ">";
            /* std::cout << ":" << val.ToString(); */
        } else {
            std::cerr << "TODO" << std::endl;
            assert(false);
        }

        std::cout << " ]" << std::endl;

    }

    //$Prefix$tid$fid$val$sid$id
    static void PrintDBIndexKey(const rocksdb::Slice& key,
                                std::shared_ptr<DBCache> cache,
                                const std::string& header = "") {
        rocksdb::Slice prefix(key.data(), PrefixSize);
        std::cout << header << "[ " << prefix.ToString() << ":";
        uint64_t tid, sid, id;
        uint8_t fid;
        rocksdb::Slice tid_slice(prefix.data() + PrefixSize, sizeof(tid));
        Serializer::Deserialize(tid_slice, tid);
        auto schema = cache->GetSchema(tid);

        rocksdb::Slice fid_slice(tid_slice.data() + sizeof(tid), sizeof(fid));
        Serializer::Deserialize(fid_slice, fid);
        uint8_t field_type;
        auto s = schema->GetFieldType(fid, field_type);
        if (!s) {
            std::cerr << "Cannot get field type of field_id" << fid << std::endl;
            return;
        }

        std::cout << tid << ":" << (int)fid;
        if (field_type == LongField::FieldTypeValue()) {
            long val;
            rocksdb::Slice val_slice(fid_slice.data()+sizeof(fid), sizeof(val));
            Serializer::Deserialize(val_slice, val);
            std::cout << ":" << val;
        } else if (field_type == FloatField::FieldTypeValue()) {
            float val;
            rocksdb::Slice val_slice(fid_slice.data()+sizeof(fid), sizeof(val));
            Serializer::Deserialize(val_slice, val);
            std::cout << ":" << val;
        } else if (field_type == StringField::FieldTypeValue()) {
            auto size = key.size() - DBTableFieldIndexPrefix.size()
                - sizeof(uint64_t) - sizeof(uint8_t) - 2 * sizeof(uint64_t);
            std::cout << ":" << rocksdb::Slice(fid_slice.data()+sizeof(fid), size).ToString();
        } else if (field_type == FloatVectorField::FieldTypeValue()) {
            /* int size = val.size() / sizeof(float); */
            /* float* data = val.data(); */
            /* std::cout << ":" */
            std::cout << ":" << "<vectors>";

        } else {
            std::cerr << "TODO" << std::endl;
            assert(false);
        }

        auto sid_addr = key.data() + key.size() - 2 * sizeof(uint64_t);
        auto id_addr = key.data() + key.size() - 1 * sizeof(uint64_t);
        rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
        rocksdb::Slice id_slice(id_addr, sizeof(id));
        Serializer::Deserialize(sid_slice, sid);
        Serializer::Deserialize(id_slice, id);

        std::cout << ":" << sid << ":" << id << " ]" << std::endl;
    }
};

class RocksDBImpl : public DBImpl {
public:
    RocksDBImpl(std::shared_ptr<rocksdb::DB> db);
    rocksdb::Status CreateTable(const std::string& table_name, const DocSchema& schema) override;
    rocksdb::Status AddDoc(const std::string& table_name, const Doc& doc) override;
    rocksdb::Status AddDocs(const std::string& table_name,
            const std::vector<std::shared_ptr<Doc>>& docs) override;

    rocksdb::Status GetLatestTableId(uint64_t tid);
    rocksdb::Status GetDoc(const std::string& table_name, long uid, std::shared_ptr<Doc> doc) override;

    rocksdb::Status GetDocs(const std::string& table_name,
            std::vector<std::shared_ptr<Doc>> docs,
            const FieldsFilter& filter) override;

    rocksdb::Status GetTables(std::vector<TablePtr>& tables) override;

    rocksdb::Status LoadField(const std::string& table_name, const std::string& field_name,
            std::vector<uint8_t>& data) override;

    rocksdb::Status LoadIndex(const std::string& table_name, const std::string& field_name);

    void Dump(bool do_print) override;

protected:

    rocksdb::Status GetTableID(const std::string& table_name, uint64_t& tid);

    rocksdb::Status GetTables(std::vector<TablePtr>& tables, const rocksdb::Snapshot* snapshot);
    rocksdb::Status LoadField(uint64_t tid, uint8_t fid, std::vector<uint8_t>& data,
            const rocksdb::Snapshot* snapshot);
    rocksdb::Status LoadField(uint64_t tid, uint64_t sid, uint8_t fid, std::vector<uint8_t>& data,
            const rocksdb::Snapshot* snapshot);

    void AddDoc(const Doc& doc, rocksdb::WriteBatch& wb,
        uint64_t& tid, uint64_t& sid, uint64_t& offset, bool& has_update);
    void Init();

    /* rocksdb::Status StoreSchema(const DocSchema& schema); */

    std::shared_ptr<rocksdb::DB> db_;
    std::shared_ptr<DBCache> db_cache_;
    rocksdb::ReadOptions rdopt_;
    rocksdb::WriteOptions wdopt_;
};

}
