#include "rocksdb_impl.h"
#include <iostream>
#include <rocksdb/db.h>
#include <set>
#include "rocksdb_util.h"
#include "utils.h"

using namespace std;

#define PRINT_STATUS(STATUS) std::cout << STATUS.ToString() << " " << __FILE__ << ":" << __LINE__ << std::endl

namespace db {


std::string TableKey(const std::string& table_name) {
    std::string key = DBTablePrefix + table_name;
    return std::move(key);
}

RocksDBImpl::RocksDBImpl(std::shared_ptr<rocksdb::DB> db)
: db_(db), db_cache_(new DBCache())
{
    Init();
}

void RocksDBImpl::Init() {
    std::string db_seq_id;
    auto s = db_->Get(rdopt_, DBTableSequenceKey, &db_seq_id);
    if (s.IsNotFound()) {
        std::cout << "Init DB first" << std::endl;
        long value = 0;
        rocksdb::Slice sval((char*)&value, sizeof(long));
        s = db_->Put(wdopt_, DBTableSequenceKey, sval);
        if (!s.ok()) {
            std::cerr << "Create DB table SequenceNumber Error: " << s.ToString() << std::endl;
            assert(false);
        }
        db_cache_->SetTid(0);
        return;
    } else if (!s.ok()) {
        std::cerr << "Get DB table SequenceNumber Error: " << s.ToString() << std::endl;
        assert(false);
    }

    uint64_t tid;
    Serializer::Deserialize(db_seq_id, tid);
    db_cache_->SetTid(tid);


    rocksdb::ReadOptions options;
    {
        std::string lower(db::DBTablePrefix);
        rocksdb::Slice l(lower);
        options.iterate_lower_bound = &l;
        rocksdb::Iterator* it = db_->NewIterator(options);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            auto key = it->key();
            if (!key.starts_with(DBTablePrefix)) {
                break;
            }
            auto val = it->value();
            std::string tk = std::string(key.data()+DBTablePrefix.size(), key.size()-DBTablePrefix.size());
            Serializer::Deserialize(val, tid);
            db_cache_->UpdateTableMapping(tid, tk);
            {
                /* KeyHelper::PrintDBTableNameKey(tk, val, db_cache_, "Initializing "); */
            }
        }
        delete it;
    }
    {
        std::string lower(DBTableSegmentNextIDPrefix);
        std::string upper(DBTableSegmentNextIDPrefix);
        uint64_t start = 0;
        Serializer::Serialize(start, lower);
        upper += db_seq_id;
        rocksdb::Slice l(lower);
        rocksdb::Slice u(upper);
        options.iterate_lower_bound = &l;
        options.iterate_upper_bound = &u;

        rocksdb::Iterator* it = db_->NewIterator(options);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            auto key = it->key();
            auto tid_addr = key.data() + DBTableMappingPrefix.size();
            uint64_t tid;
            Serializer::Deserialize(rocksdb::Slice(tid_addr, sizeof(tid)), tid);

            auto val = it->value();
            uint64_t sid, id;
            auto sid_addr = val.data();
            auto id_addr = val.data() + sizeof(sid);

            Serializer::Deserialize(rocksdb::Slice(sid_addr, sizeof(sid)), sid);
            Serializer::Deserialize(rocksdb::Slice(id_addr, sizeof(id)), id);

            db_cache_->UpdateSegMap(tid, sid);
            db_cache_->UpdateTidOffset(tid, id);
        }
        delete it;
    }

    std::string lower(db::DBTableMappingPrefix);
    std::string upper(db::DBTableMappingPrefix);
    uint64_t start = 0;
    Serializer::Serialize(start, lower);
    upper += db_seq_id;
    rocksdb::Slice l(lower);
    rocksdb::Slice u(upper);
    options.iterate_lower_bound = &l;
    options.iterate_upper_bound = &u;

    rocksdb::Iterator* it = db_->NewIterator(options);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        auto key = it->key();
        auto val = it->value();
        auto tid_addr = (uint64_t*)(key.data() + DBTableMappingPrefix.size());

        DocSchema schema;
        DocSchema::Deserialize(val, schema);
        if (!s.ok()) {
            std::cout << s.ToString() << std::endl;
        }
        Serializer::Deserialize(rocksdb::Slice((char*)tid_addr, sizeof(tid)), tid);
        db_cache_->UpdateTableMapping(tid, schema);
        /* db_cache_->UpdateTableMapping(*((uint64_t*)tid_addr), schema); */
        /* std::cout << "[[[[[" << DBTableMappingPrefix << ":" << *tid_addr; */
        /* std::cout << ", " << schema.Dump() << "]]]]]" << std::endl; */
    }

    delete it;
}

rocksdb::Status RocksDBImpl::GetDocs(const std::string& table_name,
            std::vector<std::shared_ptr<Doc>> docs,
            const FieldsFilter& filter) {
    uint64_t tid;
    auto s = db_cache_->GetTidByTname(table_name, tid);
    if (!s.ok()) {
        std::cout << s.ToString() << __func__ << ":" << __LINE__ << std::endl;
        return s;
    }

    auto schema = db_cache_->GetSchema(tid);

    auto snapshot = db_->GetSnapshot();

    std::set<std::string> filtered;
    // [Key]$Prefix:$tid:$fid$fval$sid$id -> None
    for (auto& f : filter) {
        uint8_t field_id;
        auto found = schema->GetFieldId(f.name, field_id);
        if (!found) {
            std::cerr << "Error: Cannot find field_name=" << f.name << std::endl;
            db_->ReleaseSnapshot(snapshot);
            return rocksdb::Status::InvalidArgument();
        }

        rocksdb::ReadOptions options;

        options.snapshot = snapshot;

        std::string upper(DBTableFieldIndexPrefix);
        std::string lower(DBTableFieldIndexPrefix);

        Serializer::Serialize(tid, upper);
        Serializer::Serialize(field_id, upper);
        upper.append(f.upper_bound.data(), f.upper_bound.size());
        Serializer::Serialize(std::numeric_limits<uint64_t>::min(), upper);
        Serializer::Serialize(std::numeric_limits<uint64_t>::min(), upper);

        Serializer::Serialize(tid, lower);
        Serializer::Serialize(field_id, lower);
        lower.append(f.lower_bound.data(), f.lower_bound.size());
        Serializer::Serialize(std::numeric_limits<uint64_t>::min(), lower);
        Serializer::Serialize(std::numeric_limits<uint64_t>::min(), lower);

        rocksdb::Slice l(lower);
        rocksdb::Slice u(upper);

        options.iterate_lower_bound = &l;
        options.iterate_upper_bound = &u;

        /* KeyHelper::PrintDBIndexKey(l, db_cache_, "LOWER_BOUND"); */
        /* KeyHelper::PrintDBIndexKey(u, db_cache_, "UPPER_BOUND"); */

        rocksdb::Iterator* it = db_->NewIterator(options);

        size_t items = 0;
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            if (items++ >= f.number) break;
            auto key = it->key();
            std::string location(key.data() + key.size() - 2 * sizeof(uint64_t), 2 * sizeof(uint64_t));
            Serializer::Serialize(field_id, location);
            filtered.insert(std::move(location));
        }

        delete it;
    }

    std::cout << "filtered size=" << filtered.size() << std::endl;
    // $location = $sid$id$fid
    std::string uid;
    rocksdb::ReadOptions options;
    options.snapshot = snapshot;

    for (auto& location: filtered) {
        std::string key(DBTableFieldValuePrefix);
        Serializer::Serialize(tid, key);
        key += location;
        s = db_->Get(options, key, &uid);
        assert(s.ok());
        long luid;
        Serializer::Deserialize(uid, luid);
        auto doc = std::make_shared<Doc>(Helper::NewPK(luid), schema);
        docs.push_back(doc);
        /* KeyHelper::PrintDBFieldValueKeyValue(key, uid, db_cache_); */
    }
    std::cout << "docs size=" << docs.size() << std::endl;

    /* for (auto& doc : docs) { */
    /*     std::cout << "doc : " << doc->UID() << std::endl; */
    /* } */

    db_->ReleaseSnapshot(snapshot);

    return rocksdb::Status::OK();
}

rocksdb::Status RocksDBImpl::GetDoc(const std::string& table_name, long uid, std::shared_ptr<Doc> doc) {
    uint64_t tid;
    auto s = db_cache_->GetTidByTname(table_name, tid);
    if (!s.ok()) {
        std::cout << s.ToString() << __func__ << ":" << __LINE__ << std::endl;
        return s;
    }

    std::string key(DBTableUidIdMappingPrefix);
    Serializer::Serialize(tid, key);
    Serializer::Serialize(uid, key);

    std::string sid_id;
    s = db_->Get(rdopt_, key, &sid_id);
    if (!s.ok()) {
        std::cout << s.ToString() << __func__ << ":" << __LINE__ << std::endl;
        return s;
    }

    auto schema = db_cache_->GetSchema(tid);
    doc.reset(new Doc(Helper::NewPK(uid), schema));

    //$Prefix:$tid$sid$id$fid ==> $fval
    rocksdb::ReadOptions options;
    std::string lower(DBTableFieldValuePrefix);
    std::string upper(DBTableFieldValuePrefix);
    uint8_t start = 0;
    uint8_t end = std::numeric_limits<uint8_t>::max();

    Serializer::Serialize(tid, lower);
    lower += sid_id;
    Serializer::Serialize(start, lower);

    Serializer::Serialize(tid, upper);
    upper += sid_id;
    Serializer::Serialize(end, upper);

    uint8_t fid;
    uint8_t ftype;

    rocksdb::Slice l(lower);
    rocksdb::Slice u(upper);
    options.iterate_lower_bound = &l;
    options.iterate_upper_bound = &u;

    rocksdb::Iterator* it = db_->NewIterator(options);

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        auto key = it->key();
        auto val = it->value();
        auto fid_addr = key.data() + sizeof(uint64_t)*3 + PrefixSize;
        Serializer::Deserialize(rocksdb::Slice(fid_addr, sizeof(fid)), fid);
        schema->GetFieldType(fid, ftype);
        const std::string& fname = schema->GetFieldName(fid);
        if (ftype == LongField::FieldTypeValue()) {
            if (fid == 0) continue;
            long value;
            Serializer::Deserialize(val, value);
            doc->AddLongFieldValue(fname, value);
        } else if (ftype == FloatField::FieldTypeValue()) {
            float value;
            doc->AddFloatFieldValue(fname, value);
        } else if (ftype == StringField::FieldTypeValue()) {
            doc->AddStringFieldValue(fname, val.ToString());
        } else {
            std::cerr << "Not Supported: TODO" << std::endl;
            assert(false);
        }
    }

    doc->Build();

    delete it;
    std::cout << doc->Dump() << std::endl;
}

rocksdb::Status RocksDBImpl::GetTables(std::vector<TablePtr>& tables) {
    auto snapshot = db_->GetSnapshot();
    rocksdb::Status s = GetTables(tables, snapshot);
    db_->ReleaseSnapshot(snapshot);
    return s;
}

rocksdb::Status RocksDBImpl::GetTables(std::vector<TablePtr>& tables, const rocksdb::Snapshot* snapshot) {
    rocksdb::ReadOptions options;
    options.snapshot = snapshot;

    std::string upper(DBTablePrefix);
    std::string lower(DBTablePrefix);

    uint8_t next_max = std::numeric_limits<uint8_t>::max();
    Serializer::Serialize(next_max, upper);

    rocksdb::Slice l(lower);
    rocksdb::Slice u(upper);

    options.iterate_lower_bound = &l;
    options.iterate_upper_bound = &u;


    rocksdb::Iterator* it = db_->NewIterator(options);

    size_t items = 0;
    std::string table_name;
    uint64_t tid;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        items++;
        auto key = it->key();
        auto val = it->value();
        table_name.assign(key.data()+PrefixSize, key.size()-PrefixSize);
        Serializer::Deserialize(val, tid);
        tables.emplace_back(new Table{table_name, tid});
        /* KeyHelper::PrintDBTableNameKey(key, val, db_cache_); */
    }


    delete it;
    std::cout << items << " tables found" << std::endl;

    rocksdb::Status s;
    for (auto& table : tables) {
        std::string table_seg_key(DBTableCurrentSegmentPrefix);
        Serializer::Serialize(table->id, table_seg_key);
        std::string sid_str;
        s = db_->Get(rdopt_, table_seg_key, &sid_str);
        if (!s.ok()) {
            PRINT_STATUS(s);
            assert(false);
        }
        Serializer::Deserialize(sid_str, table->current_segment_id);
        std::cout << "[TABLE] " << table->name << " " << table->id << " " << table->current_segment_id << std::endl;
    }

    std::vector<std::string> data;
    /* LoadField(0, 0, 2, data, snapshot); */
    LoadField(0, 2, data, snapshot);
    return rocksdb::Status::OK();
}

rocksdb::Status RocksDBImpl::LoadField(uint64_t tid, uint8_t fid, std::vector<std::string>& data,
        const rocksdb::Snapshot* snapshot) {
    std::string table_seg_key(DBTableCurrentSegmentPrefix);
    Serializer::Serialize(tid, table_seg_key);
    std::string sid_str;
    auto s = db_->Get(rdopt_, table_seg_key, &sid_str);
    if (!s.ok()) {
        PRINT_STATUS(s);
        assert(false);
    }
    uint64_t current_sid;
    Serializer::Deserialize(sid_str, current_sid);

    for(uint64_t sid=0; sid<=current_sid; sid++) {
        s = LoadField(tid, sid, fid, data, snapshot);
        if (!s.ok()) {
            PRINT_STATUS(s);
            break;
        }
    }

    std::cout << data.size() << " total fields found" << std::endl;
    return s;
}

// TODO: Need change if using advanced_fields and advanced_doc
rocksdb::Status RocksDBImpl::LoadField(uint64_t tid, uint64_t sid, uint8_t fid,
        std::vector<std::string>& data, const rocksdb::Snapshot* snapshot) {
    rocksdb::ReadOptions options;
    if (snapshot) options.snapshot = snapshot;

    std::string upper(DBTableFieldValuePrefix);
    std::string lower(DBTableFieldValuePrefix);

    Serializer::Serialize(tid, upper);
    Serializer::Serialize(sid, upper);
    uint8_t next_max = std::numeric_limits<uint8_t>::max();
    Serializer::Serialize(next_max, upper);

    Serializer::Serialize(tid, lower);
    Serializer::Serialize(sid, lower);
    Serializer::Serialize((uint8_t)0, lower);

    rocksdb::Slice l(lower);
    rocksdb::Slice u(upper);

    options.iterate_lower_bound = &l;
    options.iterate_upper_bound = &u;


    rocksdb::Iterator* it = db_->NewIterator(options);

    size_t items = 0;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        auto key = it->key();
        auto val = it->value();
        const char* fid_val = key.data() + PrefixSize + sizeof(tid) + sizeof(sid) + sizeof(uint64_t);
        if (*fid_val != fid) continue;
        items++;
        data.emplace_back(val.data(), val.size());
        KeyHelper::PrintDBFieldValueKeyValue(key, val, db_cache_);
    }

    delete it;
    std::cout << "(" << tid << "," << sid << "): " << items << " fields found" << std::endl;
    return rocksdb::Status::OK();
}

void RocksDBImpl::Dump(bool do_print) {
    rocksdb::ReadOptions options;
    size_t count = 0;
    rocksdb::Iterator* it = db_->NewIterator(options);
    std::map<std::string, std::string> dict;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        count++;
        if (!do_print) continue;
        auto key = it->key();
        auto val = it->value();
        if (key.starts_with(DBTablePrefix)) {
            KeyHelper::PrintDBTableNameKey(key, val, db_cache_);
        } else if (key.starts_with(DBTableCurrentSegmentPrefix)) {
            KeyHelper::PrintDBCurrentSegmentKey(key, val, db_cache_);
        } else if (key.starts_with(DBTableSequenceKey)) {
            KeyHelper::PrintDBSequenceKey(key, val, db_cache_);
        } else if (key.starts_with(DBTableSegmentNextIDPrefix)) {
            KeyHelper::PrintDBSegmentNextIDKey(key, val, db_cache_);
        } else if (key.starts_with(DBTableUidIdMappingPrefix)) {
            //$Prefix$tid$uid ==> $sid$id
            KeyHelper::PrintDBUidIdMappingKey(key, val, db_cache_);
        } else if (key.starts_with(DBTableMappingPrefix)) {
            KeyHelper::PrintDBTableMappingKey(key, val, db_cache_);
        } else if (key.starts_with(DBTableFieldIndexPrefix)) {
            KeyHelper::PrintDBIndexKey(key, db_cache_);
        } else if (key.starts_with(DBTableFieldValuePrefix)) {
            KeyHelper::PrintDBFieldValueKeyValue(key, val, db_cache_);
        } else {
            std::cout << "Error: unKown key " << key.ToString() << std::endl;
        }
    }
    delete it;
    std::cout << "Found " << count << " KVs" << std::endl;
}

rocksdb::Status RocksDBImpl::AddDocs(const std::string& table_name,
        const std::vector<std::shared_ptr<Doc>>& docs) {
    auto schema = db_cache_->GetSchema(table_name);
    if (!schema) {
        std::cout << "NotFound in Cache: table_name=" << table_name << " " << __func__ << ":" << __LINE__ << std::endl;
        return rocksdb::Status::NotFound();
    }

    uint64_t tid;
    auto s = db_cache_->GetTidByTname(table_name, tid);
    if (!s.ok()) {
        std::cout << s.ToString() << __func__ << ":" << __LINE__ << std::endl;
        return s;
    }

    uint64_t sid;
    s = db_cache_->GetSegId(tid, sid);
    if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        return s;
    }

    rocksdb::WriteBatch wb;
    bool updated = false;

    uint64_t offset;
    s = db_cache_->GetTidOffset(tid, offset);

    for (auto& doc : docs) {
        AddDoc(*doc, wb, tid, sid, offset, updated);
    }

    s = db_->Write(*DefaultDBWriteOptions(), &wb);
    if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        return s;
    }

    if (updated) {
        db_cache_->UpdateSegMap(tid, sid);
    }
    db_cache_->UpdateTidOffset(tid, offset);
    return s;
}

rocksdb::Status RocksDBImpl::AddDoc(const std::string& table_name, const Doc& doc) {
    auto schema = db_cache_->GetSchema(table_name);
    if (!schema) {
        std::cout << "NotFound in Cache: table_name=" << table_name << " " << __func__ << ":" << __LINE__ << std::endl;
        return rocksdb::Status::NotFound();
    }
    if (!doc.HasBuilt()) {
        return rocksdb::Status::InvalidArgument();
    }

    uint64_t tid;
    auto s = db_cache_->GetTidByTname(table_name, tid);
    if (!s.ok()) {
        std::cout << s.ToString() << __func__ << ":" << __LINE__ << std::endl;
        return s;
    }

    uint64_t sid;
    s = db_cache_->GetSegId(tid, sid);
    if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        return s;
    }

    rocksdb::WriteBatch wb;
    bool updated = false;

    uint64_t offset;
    s = db_cache_->GetTidOffset(tid, offset);

    AddDoc(doc, wb, tid, sid, offset, updated);

    s = db_->Write(*DefaultDBWriteOptions(), &wb);
    if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        return s;
    }

    if (updated) {
        db_cache_->UpdateSegMap(tid, sid);
    }
    db_cache_->UpdateTidOffset(tid, offset);

    return  rocksdb::Status::OK();
}

void RocksDBImpl::AddDoc(const Doc& doc, rocksdb::WriteBatch& wb,
        uint64_t& tid, uint64_t& sid, uint64_t& offset, bool& has_update) {
    std::string key;
    std::string val;
    std::map<uint8_t, std::string> doc_serialized;
    doc_serialized = doc.Serialize();
    std::string addr_to_delete;
    std::string fval_to_delete;
    rocksdb::Status s;

    uint64_t sid_used;
    uint64_t offset_used = offset;
    bool has_new = true;

    for (auto& kv : doc_serialized) {
        auto& fid = kv.first;
        auto& v = kv.second;
        if (fid == 0) {
            key.assign(DBTableUidIdMappingPrefix);
            Serializer::Serialize(tid, key);
            key.append(v);
#if 1
            {
                s = db_->Get(rdopt_, key, &addr_to_delete);
                if (s.ok()) {
                    // 1. Delete UID
                    // 2. Find And Delete All Fields, TODO
                    // 3. Update all bitmap like marker for vector deletion, TODO
                    /* std::cout << "DELETE_ID[" << key << std::endl; */
                    Serializer::Deserialize(rocksdb::Slice(addr_to_delete.data(), sizeof(sid_used)), sid_used);
                    if (sid_used != sid) {
                        wb.Delete(key);
                    } else {
                        has_new = false;
                        Serializer::Deserialize(rocksdb::Slice(addr_to_delete.data() + sizeof(sid_used), sizeof(offset_used)), offset_used);
                        addr_to_delete.clear();
                    }
                }
            }
#endif

            val.clear();
            Serializer::Serialize(sid, val);
            Serializer::Serialize(offset_used, val);
            {
                /* KeyHelper::PrintDBUidIdMappingKey(key, val, db_cache_, "NEW_UID_MAPPING"); */
            }
            // $Prefix$tid$uid ==> $sid$id
            wb.Put(key, val);
        }
        {
            if(!addr_to_delete.empty()) {
                std::string field_key(DBTableFieldValuePrefix);
                Serializer::Serialize(tid, field_key);
                field_key.append(addr_to_delete);
                Serializer::Serialize(fid, field_key);
                s = db_->Get(rdopt_, field_key, &fval_to_delete);
                if (!s.ok()) {
                    std::cerr << s.ToString() << std::endl;
                    assert(false);
                }

                {
                    /* KeyHelper::PrintDBFieldValueKeyValue(field_key, "", db_cache_, "DELETE_VALUE"); */
                }
                wb.Delete(field_key);

                std::string to_delete_index_key(DBTableFieldIndexPrefix);
                Serializer::Serialize(tid, to_delete_index_key);
                Serializer::Serialize(fid, to_delete_index_key);
                to_delete_index_key.append(fval_to_delete);
                to_delete_index_key.append(addr_to_delete);

                {
                    /* KeyHelper::PrintDBIndexKey(to_delete_index_key, db_cache_, "DELETE_INDEX"); */
                }

                wb.Delete(to_delete_index_key);
            }

            // [Key]$Prefix:$tid:$sid$id$fid    [val]$fval
            std::string field_val_key;
            field_val_key.assign(DBTableFieldValuePrefix);
            Serializer::Serialize(tid, field_val_key);
            Serializer::Serialize(sid, field_val_key);
            Serializer::Serialize(offset_used, field_val_key);
            Serializer::Serialize(fid, field_val_key);
            {
                /* KeyHelper::PrintDBFieldValueKeyValue(field_val_key, v, db_cache_, "ADDING_VALUE"); */
            }
            s = wb.Put(field_val_key, v);
            if (!s.ok()) {
                std::cerr << s.ToString() << std::endl;
                assert(false);
            }

            // [Key]$Prefix:$tid:$fid$fval$sid$id -> None
            {
                key.assign(DBTableFieldIndexPrefix);
                Serializer::Serialize(tid, key);
                Serializer::Serialize(fid, key);
                key.append(v.data(), v.size());
                Serializer::Serialize(sid, key);
                Serializer::Serialize(offset_used, key);
            }

#if 1
            {
                /* KeyHelper::PrintDBIndexKey(key, db_cache_, "ADDING_INDEX"); */
            }
#endif
            val.clear();
            wb.Put(key, val);
        }
    }

    if (has_new) {
        offset++;
        if (offset >= DBTableSegmentSize) {
            sid++;
            offset = 0;
            has_update = true;
            std::string current_seg(DBTableCurrentSegmentPrefix);
            Serializer::Serialize(tid, current_seg);
            std::string v;
            Serializer::Serialize(sid, v);
            wb.Put(current_seg, v);
        }
    }

    std::string next_id_k(DBTableSegmentNextIDPrefix);
    Serializer::Serialize(tid, next_id_k);
    std::string next_id_v;
    Serializer::Serialize(sid, next_id_v);
    Serializer::Serialize(offset, next_id_v);
    wb.Put(next_id_k, next_id_v);
}

rocksdb::Status RocksDBImpl::CreateTable(const std::string& table_name, const DocSchema& schema) {
    auto table_key = TableKey(table_name);

    std::string table_key_value;

    std::stringstream ss;

    auto s = db_->Get(rdopt_, table_key, &table_key_value);
    if (s.IsNotFound()) {
        if (!schema.HasBuilt()) {
            return rocksdb::Status::InvalidArgument("DocSchema is expected to be built first!");
        }
        // TODO: create table should be thread safe
        uint64_t sid = 0;
        uint64_t id = 0;
        auto tid = db_cache_->GetTid();
        /* std::cout << "cache Current tid=" << tid << std::endl; */
        auto next_tid = tid + 1;
        rocksdb::WriteBatch wb;

        std::string tsk_val;
        Serializer::Serialize(next_tid, tsk_val);
        /* std::cout << "Updating next tid to " << next_tid << std::endl; */
        wb.Put(DBTableSequenceKey, tsk_val);

        std::string t_v;
        Serializer::Serialize(tid, t_v);
        /* std::cout << "Putting " << tid << " to " << table_key << std::endl; */
        wb.Put(table_key, t_v);

        std::string ts_k(DBTableCurrentSegmentPrefix);
        Serializer::Serialize(tid, ts_k);
        std::string ts_v;
        Serializer::Serialize(sid, ts_v);
        wb.Put(ts_k, ts_v);

        // $Prefix$tid ==> $sid$id
        std::string tss_k(DBTableSegmentNextIDPrefix);
        Serializer::Serialize(tid, tss_k);
        std::string tss_v;
        Serializer::Serialize(sid, tss_v);
        Serializer::Serialize(id, tss_v);
        wb.Put(tss_k, tss_v);

        std::string schema_serialized;
        s = schema.Serialize(schema_serialized);
        if (!s.ok()) {
            std::cerr << s.ToString() << std::endl;
            assert(false);
        }
        std::string tm_k(DBTableMappingPrefix);
        Serializer::Serialize(tid, tm_k);
        wb.Put(tm_k, schema_serialized);

        s = db_->Write(*DefaultDBWriteOptions(), &wb);
        if (!s.ok()) {
            std::cerr << s.ToString() << std::endl;
            assert(false);
        }

        db_cache_->UpdateSegMap(tid, 0);
        db_cache_->UpdateTidOffset(tid, id);
        auto s = db_cache_->GetSegId(tid, sid);
        if (!s.ok()) {
            std::cout << "tid=" << tid  << " sid=" << sid << std::endl;
            std::cerr << s.ToString() << std::endl;
            assert(false);
        }
        /* std::cout << "segid=" << sid << std::endl; */
        db_cache_->SetTid(next_tid, &table_name, &schema);
        db_cache_->UpdateTableMapping(tid, schema);

        /* demo::read_all(db_); */

    } else if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        assert(false);
    }

    return rocksdb::Status::OK();
}

}
