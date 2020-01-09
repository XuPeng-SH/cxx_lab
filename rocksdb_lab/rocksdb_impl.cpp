#include "rocksdb_impl.h"
#include <iostream>
#include <rocksdb/db.h>
#include "rocksdb_util.h"
#include "utils.h"

using namespace std;

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

    uint64_t tid = *(uint64_t*)(db_seq_id.data());
    db_cache_->SetTid(tid);

    /* s = db_->Get(rdopt_, DB) */

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
            db_cache_->UpdateTableMapping(*(uint64_t*)(val.data()), tk);
            std::cout << "Found k=" << tk << " tid=" << *(uint64_t*)(val.data()) << " " << __func__ << ":" << __LINE__ << std::endl;
        }
        delete it;
    }
    {
        std::string lower(DBTableSegmentNextIDPrefix);
        std::string upper(DBTableSegmentNextIDPrefix);
        uint64_t start = 0;
        Serializer::SerializeNumeric(start, lower);
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
            Serializer::DeserializeNumeric(rocksdb::Slice(tid_addr, sizeof(tid)), tid);

            auto val = it->value();
            uint64_t sid, id;
            auto sid_addr = val.data();
            auto id_addr = val.data() + sizeof(sid);

            Serializer::DeserializeNumeric(rocksdb::Slice(sid_addr, sizeof(sid)), sid);
            Serializer::DeserializeNumeric(rocksdb::Slice(id_addr, sizeof(id)), id);

            db_cache_->UpdateSegMap(tid, sid);
            db_cache_->UpdateTidOffset(tid, id);
        }
        delete it;
    }

    std::string lower(db::DBTableMappingPrefix);
    std::string upper(db::DBTableMappingPrefix);
    uint64_t start = 0;
    Serializer::SerializeNumeric(start, lower);
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
        db_cache_->UpdateTableMapping(*((uint64_t*)tid_addr), schema);
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

    for (auto& f : filter) {
        rocksdb::ReadOptions options;
        options.iterate_upper_bound = f.upper_bound;
        options.iterate_lower_bound = f.lower_bound;

        rocksdb::Iterator* it = db_->NewIterator(options);

        size_t items = 0;
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            if (items >= f.number) break;
            auto key = it->key();

        }

        delete it;
    }

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
    Serializer::SerializeNumeric(tid, key);
    Serializer::SerializeNumeric(uid, key);

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

    Serializer::SerializeNumeric(tid, lower);
    lower += sid_id;
    Serializer::SerializeNumeric(start, lower);

    Serializer::SerializeNumeric(tid, upper);
    upper += sid_id;
    Serializer::SerializeNumeric(end, upper);

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
        Serializer::DeserializeNumeric(rocksdb::Slice(fid_addr, sizeof(fid)), fid);
        schema->GetFieldType(fid, ftype);
        const std::string& fname = schema->GetFieldName(fid);
        if (ftype == LongField::FieldTypeValue()) {
            long value;
            Serializer::DeserializeNumeric(val, value);
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
            uint64_t tid;
            Serializer::DeserializeNumeric(val, tid);
            std::cout << "[" << key.ToString() << ", " << tid << "]" << std::endl;

        } else if (key.starts_with(DBTableCurrentSegmentPrefix)) {
            auto tid_addr = key.data() + DBTableCurrentSegmentPrefix.size();
            uint64_t tid;
            rocksdb::Slice tid_sli(tid_addr, sizeof(tid));
            Serializer::DeserializeNumeric(tid_sli, tid);

            uint64_t sid;
            Serializer::DeserializeNumeric(val, sid);

            std::cout << "[" << DBTableCurrentSegmentPrefix << ":" << tid << ", " << tid << "]" << std::endl;

        } else if (key.starts_with(DBTableSequenceKey)) {
            uint64_t tid;
            Serializer::DeserializeNumeric(val, tid);

            std::cout << "[" << DBTableSequenceKey << ", " << tid << "]" << std::endl;

        } else if (key.starts_with(DBTableSegmentNextIDPrefix)) {
            uint64_t tid, sid, id;
            auto tid_addr = key.data() + DBTableSegmentNextIDPrefix.size();
            auto sid_addr = val.data();
            auto id_addr = val.data() + sizeof(uint64_t);
            rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
            rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
            rocksdb::Slice id_slice(id_addr, sizeof(id));

            Serializer::DeserializeNumeric(tid_slice, tid);
            Serializer::DeserializeNumeric(sid_slice, sid);
            Serializer::DeserializeNumeric(id_slice, id);

            std::cout << "[" << DBTableSegmentNextIDPrefix << tid << ", " << sid;
            std::cout << ":" << id << "]" << std::endl;

        } else if (key.starts_with(DBTableUidIdMappingPrefix)) {
            //$Prefix$tid$uid ==> $sid$id
            uint64_t tid, sid, id;
            long uid;
            auto tid_addr = key.data() + DBTableUidIdMappingPrefix.size();
            auto uid_addr = tid_addr + sizeof(tid);
            auto sid_addr = val.data();
            auto id_addr = sid_addr + sizeof(sid);

            rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
            rocksdb::Slice uid_slice(uid_addr, sizeof(uid));
            rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
            rocksdb::Slice id_slice(id_addr, sizeof(id));

            Serializer::DeserializeNumeric(tid_slice, tid);
            Serializer::DeserializeNumeric(sid_slice, sid);
            Serializer::DeserializeNumeric(uid_slice, uid);
            Serializer::DeserializeNumeric(id_slice, id);

            std::cout << "[" << DBTableUidIdMappingPrefix << ":" << tid << ":" << uid;
            std::cout << ", " << sid << ":" << id << "]" << std::endl;

        } else if (key.starts_with(DBTableMappingPrefix)) {
            uint64_t tid;
            auto tid_addr = key.data() + DBTableMappingPrefix.size();
            rocksdb::Slice tid_slice(tid_addr, sizeof(tid));

            DocSchema schema;
            auto s = DocSchema::Deserialize(val, schema);
            if (!s.ok()) {
                std::cout << s.ToString() << std::endl;
            }
            std::cout << "[" << DBTableMappingPrefix << ":" << tid << "]" << std::endl;
            /* std::cout << "[" << DBTableMappingPrefix << ":" << *tid_addr; */
            /* std::cout << ", " << schema.Dump() << "]" << std::endl; */

        } else if (key.starts_with(DBTableFieldIndexPrefix)) {
            // [Key]$Prefix:$tid:$fid$fval [Val]$sid$id
            uint64_t tid;
            auto tid_addr = key.data() + DBTableFieldIndexPrefix.size();
            rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
            Serializer::DeserializeNumeric(tid_slice, tid);
            auto schema = db_cache_->GetSchema(tid);

            uint8_t fid;
            auto fid_addr = (char*)(tid_addr) + sizeof(uint64_t);
            auto fval_addr = (char*)((char*)(tid_addr) + sizeof(uint64_t) + sizeof(uint8_t));

            rocksdb::Slice fid_slice(fid_addr, sizeof(fid));
            Serializer::DeserializeNumeric(fid_slice, fid);

            std::cout << "[" << DBTableFieldIndexPrefix << ":" << tid << ":" << (int)fid << ":";
            uint8_t ftype;
            auto s = schema->GetFieldType(fid, ftype);
            if (!s) {
                std::cerr << "Cannot get field type of field_id" << fid << std::endl;
                return;
            }

            if (ftype == LongField::FieldTypeValue()) {
                std::cout << *(long*)(fval_addr);
            } else if (ftype == FloatField::FieldTypeValue()) {
                std::cout << *(float*)(fval_addr);
            } else if (ftype == StringField::FieldTypeValue()) {
                auto size = key.size() - DBTableFieldIndexPrefix.size()
                    - sizeof(uint64_t) - sizeof(uint8_t) - 2 * sizeof(uint64_t);
                std::cout << rocksdb::Slice(fval_addr, size).ToString();
            } else {
                std::cerr << "TODO" << std::endl;
                assert(false);
            }

            uint64_t sid, id;
            auto sid_addr = key.data() + key.size() - 2 * sizeof(uint64_t);
            auto id_addr = key.data() + key.size() - 1 * sizeof(uint64_t);
            rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
            rocksdb::Slice id_slice(id_addr, sizeof(id));
            Serializer::DeserializeNumeric(sid_slice, sid);
            Serializer::DeserializeNumeric(id_slice, id);

            std::cout << ":" << sid << ":" << id << "]" << std::endl;

        } else if (key.starts_with(DBTableFieldValuePrefix)) {
            // [Key]$Prefix:$tid:$sid$id$fid    [val]$fval
            uint64_t tid, sid, offset;
            uint8_t fid;
            auto tid_addr = key.data() + DBTableFieldValuePrefix.size();
            auto sid_addr = key.data() + DBTableFieldValuePrefix.size() + sizeof(uint64_t);
            auto offset_addr = key.data() + DBTableFieldValuePrefix.size() + 2*sizeof(uint64_t);
            auto fid_addr = (char*)(tid_addr) + 3*sizeof(uint64_t);

            rocksdb::Slice tid_slice(tid_addr, sizeof(tid));
            rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
            rocksdb::Slice offset_slice(offset_addr, sizeof(offset));
            rocksdb::Slice fid_slice(fid_addr, sizeof(fid));

            Serializer::DeserializeNumeric(tid_slice, tid);
            Serializer::DeserializeNumeric(sid_slice, sid);
            Serializer::DeserializeNumeric(offset_slice, offset);
            Serializer::DeserializeNumeric(fid_slice, fid);

            std::cout << "[" << DBTableFieldValuePrefix << ":" << tid << ":" << sid << ":" << offset << ":" << (int)fid;
            std::cout << ":<>]" << std::endl;

        } else {
            std::cout << "Error: unKown key " << key.ToString() << std::endl;
        }
    }
    delete it;
    std::cout << "Found " << count << " KVs" << std::endl;
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

    std::string key;
    std::string val;
    uint64_t sid;
    s = db_cache_->GetSegId(tid, sid);
    if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        return s;
    }

    rocksdb::WriteBatch wb;

    uint64_t offset;
    s = db_cache_->GetTidOffset(tid, offset);
    std::map<uint8_t, std::string> doc_serialized;
    doc_serialized = doc.Serialize();
    std::string addr_to_delete;
    std::string fval_to_delete;

    for (auto& kv : doc_serialized) {
        auto& fid = kv.first;
        auto& v = kv.second;
        if (fid == 0) {
            key.assign(DBTableUidIdMappingPrefix);
            Serializer::SerializeNumeric(tid, key);
            key.append(v);

            {
                s = db_->Get(rdopt_, key, &addr_to_delete);
                if (s.ok()) {
                    // 1. Delete UID
                    // 2. Find And Delete All Fields, TODO
                    // 3. Update all bitmap like marker for vector deletion, TODO
                    /* std::cout << "DELETE_ID[" << key << std::endl; */
                    wb.Delete(key);
                }
            }

            val.clear();
            Serializer::SerializeNumeric(sid, val);
            Serializer::SerializeNumeric(offset, val);
            {
                long uid;
                Serializer::DeserializeNumeric(v, uid);
                std::cout << "NEW_UID_MAPPING[" << DBTableUidIdMappingPrefix << ":" << tid << ":" << uid;
                std::cout << ", " << sid << ":" << offset << "]" << std::endl;
            }
            // $Prefix$tid$uid ==> $sid$id
            wb.Put(key, val);
        } else {
            if(!addr_to_delete.empty()) {
                std::string field_key(DBTableFieldValuePrefix);
                Serializer::SerializeNumeric(tid, field_key);
                field_key.append(addr_to_delete);
                Serializer::SerializeNumeric(fid, field_key);
                s = db_->Get(rdopt_, field_key, &fval_to_delete);
                if (!s.ok()) {
                    std::cerr << s.ToString() << std::endl;
                    assert(false);
                }

                {
                    uint64_t sid, offset;
                    auto sid_addr = addr_to_delete.data();
                    auto offset_addr = sid_addr + 1;
                    rocksdb::Slice sid_slice(sid_addr, sizeof(sid));
                    rocksdb::Slice offset_slice(offset_addr, sizeof(offset));
                    Serializer::DeserializeNumeric(sid_slice, sid);
                    Serializer::DeserializeNumeric(offset_slice, offset);
                    std::cout << "DELETE_VALUE[" << DBTableFieldValuePrefix << ":" << tid << ":" << sid << ":" << offset << ":" << (int)fid << "]" << std::endl;

                }
                wb.Delete(field_key);

                std::string to_delete_index_key(DBTableFieldIndexPrefix);
                Serializer::SerializeNumeric(tid, to_delete_index_key);
                Serializer::SerializeNumeric(fid, to_delete_index_key);
                to_delete_index_key.append(fval_to_delete);
                to_delete_index_key.append(addr_to_delete);

                {
                    std::cout << "DELETE_INDEX[" << DBTableFieldIndexPrefix << ":" << tid << ":" << (int)fid;
                    if (fid==1)
                        std::cout <<  ":" << *(long*)(fval_to_delete.data()) << "]" << std::endl;
                    else
                        std::cout <<  ":" << fval_to_delete << "]" << std::endl;
                }
                wb.Delete(to_delete_index_key);
            }

            // [Key]$Prefix:$tid:$sid$id$fid    [val]$fval
            std::string field_val_key;
            field_val_key.assign(DBTableFieldValuePrefix);
            Serializer::SerializeNumeric(tid, field_val_key);
            Serializer::SerializeNumeric(sid, field_val_key);
            Serializer::SerializeNumeric(offset, field_val_key);
            Serializer::SerializeNumeric(fid, field_val_key);
            {

                std::cout << "ADDING_VALUE[" << DBTableFieldValuePrefix << ":" << tid << ":" << sid << ":" << offset << ":" << (int)fid << "," << v << "]" << std::endl;
            }
            s = wb.Put(field_val_key, v);
            if (!s.ok()) {
                std::cerr << s.ToString() << std::endl;
                assert(false);
            }

            // [Key]$Prefix:$tid:$fid$fval$sid$id -> None
            key.assign(DBTableFieldIndexPrefix);
            Serializer::SerializeNumeric(tid, key);
            Serializer::SerializeNumeric(fid, key);
            key.append(v.data(), v.size());
            Serializer::SerializeNumeric(sid, key);
            Serializer::SerializeNumeric(offset, key);

#if 1
            {
                uint8_t field_type;
                auto fval_addr = v.data();
                schema->GetFieldType(fid, field_type);
                std::cout << "ADDING_INDEX[" << DBTableFieldIndexPrefix << ":" << tid << ":" << (int)fid << ":";
                if (field_type == LongField::FieldTypeValue()) {
                    long vv;
                    Serializer::DeserializeNumeric(rocksdb::Slice(fval_addr, sizeof(vv)), vv);
                    std::cout << vv;
                } else if (field_type == FloatField::FieldTypeValue()) {
                    float vv;
                    Serializer::DeserializeNumeric(rocksdb::Slice(fval_addr, sizeof(vv)), vv);
                    std::cout << vv;
                } else if (field_type == StringField::FieldTypeValue()) {
                    auto size = key.size() - DBTableFieldIndexPrefix.size()
                        - sizeof(uint64_t) - sizeof(uint8_t) - 2 * sizeof(uint64_t);
                    std::cout << rocksdb::Slice(fval_addr, size).ToString();
                } else {
                    std::cerr << "TODO" << std::endl;
                    assert(false);
                }

                /* std::cout  << v */
                std::cout  << ":" << sid << ":" << offset << "]" << std::endl;
            }
#endif
            val.clear();
            wb.Put(key, val);
        }
    }

    bool updated = false;
    offset++;
    if (offset >= DBTableSegmentSize) {
        sid++;
        offset = 0;
        updated = true;
        std::string current_seg(DBTableCurrentSegmentPrefix);
        Serializer::SerializeNumeric(tid, current_seg);
        std::string v;
        Serializer::SerializeNumeric(sid, v);
        wb.Put(current_seg, v);
    }

    std::string next_id_k(DBTableSegmentNextIDPrefix);
    Serializer::SerializeNumeric(tid, next_id_k);
    std::string next_id_v;
    Serializer::SerializeNumeric(sid, next_id_v);
    Serializer::SerializeNumeric(offset, next_id_v);
    wb.Put(next_id_k, next_id_v);

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
        auto next_tid = tid + 1;
        rocksdb::WriteBatch wb;

        std::string tsk_val;
        Serializer::SerializeNumeric(next_tid, tsk_val);
        std::cout << "Updating next tid to " << next_tid << std::endl;
        wb.Put(DBTableSequenceKey, tsk_val);

        std::string t_v;
        Serializer::SerializeNumeric(tid, t_v);
        std::cout << "Putting " << tid << " to " << table_key << std::endl;
        wb.Put(table_key, t_v);

        std::string ts_k(DBTableCurrentSegmentPrefix);
        Serializer::SerializeNumeric(tid, ts_k);
        std::string ts_v;
        Serializer::SerializeNumeric(sid, ts_v);
        wb.Put(ts_k, ts_v);

        // $Prefix$tid ==> $sid$id
        std::string tss_k(DBTableSegmentNextIDPrefix);
        Serializer::SerializeNumeric(tid, tss_k);
        std::string tss_v;
        Serializer::SerializeNumeric(sid, tss_v);
        Serializer::SerializeNumeric(id, tss_v);
        wb.Put(tss_k, tss_v);

        std::string schema_serialized;
        s = schema.Serialize(schema_serialized);
        if (!s.ok()) {
            std::cerr << s.ToString() << std::endl;
            assert(false);
        }
        std::string tm_k(DBTableMappingPrefix);
        Serializer::SerializeNumeric(tid, tm_k);
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
        /* demo::read_all(db_); */

    } else if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        assert(false);
    }

    return rocksdb::Status::OK();
}

}
