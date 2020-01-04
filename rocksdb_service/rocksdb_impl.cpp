#include "rocksdb_impl.h"
#include <iostream>
#include <rocksdb/db.h>
#include "rocksdb_util.h"

using namespace std;

namespace db {

static constexpr const size_t DBTableSegmentSize = 500000; // TODO: Support Customized size

// (K,V) = (TSK, 1)        ==> The next Table ID to be allocated is 1
// (K,V) = (TSK, 28)       ==> The next Table ID to be allocated is 28
static const std::string DBTableSequenceKey = "TSK";

// (K,V) = (T:t0, 0)        ==> Table Name 't0' is assigned with Table ID 0
// (K,V) = (T:new_t, 1)     ==> Table Name 'new_t' is assigned with Table ID 1
static const std::string DBTablePrefix = "T:";

// (K,V) = (TS:0, (0,0))    ==> Table ID 0 current effective segment is 0, offset is 0
// (K,V) = (TS:0, (1,1000)) ==> Table ID 0 current effective segment is 1, offset is 1000
// (K,V) = (TS:2, (5,2000)) ==> Table ID 2 current effective segment is 5, offset is 2000
static const std::string DBTableCurrentSegmentPrefix = "TS:";

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

    std::cout << "Latest TID=" << *((uint64_t*)db_seq_id.data()) << std::endl;
    db_cache_->SetTid(*((uint64_t*)db_seq_id.data()));
    demo::read_all(db_);
}

rocksdb::Status RocksDBImpl::CreateTable(const std::string& table_name, const DocSchema& schema) {
    auto table_key = TableKey(table_name);
    cout << "RocksDBImpl::CreateTable: " << table_key << endl;

    std::string table_key_value;

    std::stringstream ss;

    auto s = db_->Get(rdopt_, table_key, &table_key_value);
    if (s.IsNotFound()) {
        // TODO: create table should be thread safe
        auto tid = db_cache_->GetTid();
        auto next_tid = tid + 1;
        rocksdb::WriteBatch wb;

        /* std::string tsk_val; */
        /* tsk_val.append((char*)&next_tid, sizeof(next_tid)); */
        /* wb.Put(DBTableSequenceKey, tsk_val); */
        ss << next_tid;
        wb.Put(DBTableSequenceKey, ss.str());
        ss.str("");

        /* std::string t_v; */
        /* t_v.append((char*)&tid, sizeof(tid)); */
        /* wb.Put(table_key, t_v); */
        ss << tid;
        wb.Put(table_key, ss.str());
        ss.str("");

        /* std::string ts_k(DBTableCurrentSegmentPrefix); */
        /* ts_k.append((char*)&tid, sizeof(tid)); */
        /* std::string ts_v; */
        /* std::array<uint64_t, 2> ts_arr = {tid, 0}; */
        /* ts_v.append((char*)&ts_arr, sizeof(tid)*ts_arr.size()); */
        /* wb.Put(ts_k, ts_v); */
        ss << DBTableCurrentSegmentPrefix << tid;
        auto ts_k = ss.str();
        ss.str("");
        ss << tid << "," << 0;
        wb.Put(ts_k, ss.str());
        ss.str("");

        s = db_->Write(*DefaultDBWriteOptions(), &wb);
        if (!s.ok()) {
            std::cerr << s.ToString() << std::endl;
            assert(false);
        }

        db_cache_->UpdateSegMap(tid, 0);
        std::cout << "tid=" << tid << std::endl;
        uint64_t sid;
        auto s = db_cache_->GetSegId(tid, sid);
        if (!s.ok()) {
            std::cerr << s.ToString() << std::endl;
            assert(false);
        }
        std::cout << "segid=" << sid << std::endl;
        db_cache_->SetTid(next_tid);
        demo::read_all(db_);

    } else if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        assert(false);
    }

    return rocksdb::Status::OK();
}

}
