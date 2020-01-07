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

    db_cache_->SetTid(*((uint64_t*)db_seq_id.data()));

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
    /* demo::read_all(db_, nullptr, true); */
}


rocksdb::Status RocksDBImpl::AddDoc(const std::string& table_name, const Doc& doc) {
    auto schema = db_cache_->GetSchemaByTname(table_name);
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
    uint64_t _id_field_id = 0;
    uint64_t age_field_id = 1;
    uint64_t LONG_TYPE = 1;

    std::string id_field_key(DBTableFieldValuePrefix);
    id_field_key.append((char*)(&tid), sizeof(tid));
    id_field_key.append((char*)(&_id_field_id), sizeof(uint64_t));
    id_field_key.append((char*)(&LONG_TYPE), sizeof(uint64_t));
    id_field_key.append((char*)(&LONG_TYPE), sizeof(uint64_t));

    return  rocksdb::Status::OK();
}

rocksdb::Status RocksDBImpl::CreateTable(const std::string& table_name, const DocSchema& schema) {
    auto table_key = TableKey(table_name);
    /* cout << "RocksDBImpl::CreateTable: " << table_key << endl; */

    std::string table_key_value;

    std::stringstream ss;

    auto s = db_->Get(rdopt_, table_key, &table_key_value);
    if (s.IsNotFound()) {
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

        std::string tss_k(DBTableSegmentNextIDPrefix);
        tss_k.append((char*)&tid, sizeof(tid));
        tss_k.append((char*)&sid, sizeof(sid));
        std::string tss_v;
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
