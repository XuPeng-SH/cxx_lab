#include "rocksdb_impl.h"
#include <iostream>
#include <rocksdb/db.h>
#include "rocksdb_util.h"

using namespace std;

namespace db {


std::string TableKey(const std::string& table_name) {
    std::string key = DBTablePrefix + table_name;
    return std::move(key);
}

/* struct A { */
/*  A(int a) : a_(a) { std::cout << "C" << std::endl; } */
/*  A(const A& o) { a_ = o.a_; std::cout << "C&" << std::endl; } */
/*  A(A&& o) { a_ = std::move(o.a_); std::cout << "C&&" << std::endl; } */
/*  int a_; */
/* }; */

/* A CallA() { */
/*     A a(2); */
/*     return std::move(a); */
/* } */


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
        lower.append((char*)&start, sizeof(start));
        upper += db_seq_id;
        rocksdb::Slice l(lower);
        rocksdb::Slice u(upper);
        options.iterate_lower_bound = &l;
        options.iterate_upper_bound = &u;

        rocksdb::Iterator* it = db_->NewIterator(options);
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            auto key = it->key();
            auto tid = *(uint64_t*)(key.data() + DBTableMappingPrefix.size());

            auto val = it->value();
            uint64_t sid = *(uint64_t*)(val.data());
            uint64_t id = *(uint64_t*)(val.data() + sizeof(sid));

            db_cache_->UpdateSegMap(tid, sid);
            db_cache_->UpdateTidOffset(tid, id);
        }
        delete it;
    }

    std::string lower(db::DBTableMappingPrefix);
    std::string upper(db::DBTableMappingPrefix);
    uint64_t start = 0;
    lower.append((char*)&start, sizeof(start));
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
        auto s = Serializer::DeserializeDocSchema(val, schema);
        if (!s.ok()) {
            std::cout << s.ToString() << std::endl;
        }
        db_cache_->UpdateTableMapping(*((uint64_t*)tid_addr), schema);
        /* std::cout << "[[[[[" << DBTableMappingPrefix << ":" << *tid_addr; */
        /* std::cout << ", " << schema.Dump() << "]]]]]" << std::endl; */
    }

    delete it;
    /* std::cout << "Start read_all ... " << std::endl; */
    Dump(true);
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
            std::cout << "[" << key.ToString() << ", " << *(uint64_t*)(it->value().data()) << "]" << std::endl;
        } else if (key.starts_with(DBTableCurrentSegmentPrefix)) {
            auto sid_addr = key.data() + DBTableCurrentSegmentPrefix.size();
            std::cout << "[" << DBTableCurrentSegmentPrefix << *(uint64_t*)(sid_addr) << ", " << *(uint64_t*)(it->value().data()) << "]" << std::endl;
        } else if (key.starts_with(DBTableSequenceKey)) {
            std::cout << "[" << DBTableSequenceKey << ", " << *(uint64_t*)(it->value().data()) << "]" << std::endl;
        } else if (key.starts_with(DBTableSegmentNextIDPrefix)) {
            auto tid_addr = (uint64_t*)(key.data() + DBTableSegmentNextIDPrefix.size());
            /* auto sid_addr = (uint64_t*)(key.data() + DBTableSegmentNextIDPrefix.size() + 1 + sizeof(uint64_t)); */
            auto sid_addr = (uint64_t*)(val.data());
            auto id_addr = (uint64_t*)(val.data() + sizeof(uint64_t));
            std::cout << "[" << DBTableSegmentNextIDPrefix << *tid_addr << ", " << *sid_addr;
            std::cout << ":" << *id_addr << "]" << std::endl;
        } else if (key.starts_with(DBTableUidIdMappingPrefix)) {
            auto tid_addr = (uint64_t*)(key.data() + DBTableUidIdMappingPrefix.size());
            auto uid_addr = (uint64_t*)(key.data() + DBTableUidIdMappingPrefix.size() + sizeof(uint64_t));

            auto sid_addr = (uint64_t*)(val.data());
            auto id_addr = (uint64_t*)(val.data() + sizeof(uint64_t));

            std::cout << "[" << DBTableUidIdMappingPrefix << ":" << *tid_addr << ":" << *uid_addr;
            std::cout << ", " << *sid_addr << ":" << *id_addr << "]" << std::endl;
        } else if (key.starts_with(DBTableMappingPrefix)) {
            auto tid_addr = (uint64_t*)(key.data() + DBTableMappingPrefix.size());

            DocSchema schema;
            auto s = Serializer::DeserializeDocSchema(val, schema);
            if (!s.ok()) {
                std::cout << s.ToString() << std::endl;
            }
            /* std::cout << "[" << DBTableMappingPrefix << ":" << *tid_addr; */
            /* std::cout << ", " << schema.Dump() << "]" << std::endl; */
        } else if (key.starts_with(DBTableFieldValuePrefix)) {
            // [Key]$Prefix:$tid:$fid$fval [Val]$sid$id
            /* continue; */
            auto tid_addr = (uint64_t*)(key.data() + DBTableFieldValuePrefix.size());
            auto schema = db_cache_->GetSchema(*tid_addr);
            auto fid_addr = (uint8_t*)((char*)(tid_addr) + sizeof(uint64_t));
            auto fval_addr = (char*)((char*)(tid_addr) + sizeof(uint64_t) + sizeof(uint8_t));

            std::cout << "[" << DBTableFieldValuePrefix << ":" << *tid_addr << ":" << (int)*fid_addr << ":";
            uint8_t ftype;
            auto s = schema->GetFieldType(*fid_addr, ftype);
            if (!s) {
                std::cerr << "Cannot get field type of field_id" << *fid_addr << std::endl;
                return;
            }

            if (ftype == LongField::FieldTypeValue()) {
                std::cout << *(long*)(fval_addr);
            } else if (ftype == FloatField::FieldTypeValue()) {
                std::cout << *(float*)(fval_addr);
            } else if (ftype == StringField::FieldTypeValue()) {
                auto size = key.size() - DBTableFieldValuePrefix.size()
                    - sizeof(uint64_t) - sizeof(uint8_t) - 2 * sizeof(uint64_t);
                std::cout << rocksdb::Slice(fval_addr, size).ToString();
            } else {
                std::cerr << "TODO" << std::endl;
                assert(false);
            }

            auto sid_addr = (uint64_t*)(key.data() + key.size() - 2 * sizeof(uint64_t));
            auto id_addr = (uint64_t*)(key.data() + key.size() - 1 * sizeof(uint64_t));
            std::cout << ":" << *sid_addr << ":" << *id_addr << "]" << std::endl;
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
    Serializer::SerializeDoc(doc, doc_serialized);
    for (auto& kv : doc_serialized) {
        auto& fid = kv.first;
        auto& v = kv.second;
        key.assign(DBTableFieldValuePrefix);
        key.append((char*)(&tid), sizeof(tid));
        key.append((char*)(&fid), sizeof(uint8_t));
        key.append(v.data(), v.size());
        key.append((char*)(&sid), sizeof(sid));
        key.append((char*)(&offset), sizeof(offset));

        val.clear();

        wb.Put(key, val);
    }

    bool updated = false;
    offset++;
    if (offset >= DBTableSegmentSize) {
        sid++;
        offset = 0;
        updated = true;
        std::string next_tid;
        next_tid.append((char*)(&tid), sizeof(tid));
        wb.Put(DBTableSequenceKey, next_tid);

        std::string current_seg(DBTableCurrentSegmentPrefix);
        current_seg.append((char*)(&tid), sizeof(tid));
        std::string v;
        v.append((char*)&sid, sizeof(sid));
        wb.Put(current_seg, v);
    }

    std::string next_id_k(DBTableSegmentNextIDPrefix);
    next_id_k.append((char*)&tid, sizeof(tid));
    std::string next_id_v;
    next_id_v.append((char*)&sid, sizeof(sid));
    next_id_v.append((char*)&offset, sizeof(offset));
    wb.Put(next_id_k, next_id_v);

    s = db_->Write(*DefaultDBWriteOptions(), &wb);
    if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        return s;
    }

    if (updated) {
        db_cache_->UpdateSegMap(tid, sid);
        db_cache_->UpdateTidOffset(tid, offset);
    }

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
        tsk_val.append((char*)&next_tid, sizeof(next_tid));
        wb.Put(DBTableSequenceKey, tsk_val);

        std::string t_v;
        t_v.append((char*)&tid, sizeof(tid));
        std::cout << "Putting " << tid << " to " << table_key << std::endl;
        wb.Put(table_key, t_v);

        std::string ts_k(DBTableCurrentSegmentPrefix);
        ts_k.append((char*)&tid, sizeof(tid));
        std::string ts_v;
        ts_v.append((char*)&sid, sizeof(sid));
        wb.Put(ts_k, ts_v);

        // $Prefix$tid ==> $sid$id
        std::string tss_k(DBTableSegmentNextIDPrefix);
        tss_k.append((char*)&tid, sizeof(tid));
        std::string tss_v;
        tss_v.append((char*)&sid, sizeof(sid));
        tss_v.append((char*)&id, sizeof(id));
        wb.Put(tss_k, tss_v);

        std::string schema_serialized;
        s = Serializer::SerializeDocSchema(schema, schema_serialized);
        if (!s.ok()) {
            std::cerr << s.ToString() << std::endl;
            assert(false);
        }
        std::string tm_k(DBTableMappingPrefix);
        tm_k.append((char*)&tid, sizeof(tid));
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
