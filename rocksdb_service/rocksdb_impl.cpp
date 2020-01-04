#include "rocksdb_impl.h"
#include <iostream>
#include <rocksdb/db.h>

using namespace std;

namespace db {

static const std::string DBTableSequenceKey = "TSK";

static const std::string DBTablePrefix = "T:";

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
}

rocksdb::Status RocksDBImpl::CreateTable(const std::string& table_name, const DocSchema& schema) {
    auto table_key = TableKey(table_name);
    cout << "RocksDBImpl::CreateTable: " << table_key << endl;

    std::string table_key_value;

    auto s = db_->Get(rdopt_, table_key, &table_key_value);
    if (s.IsNotFound()) {
        auto tid = db_cache_->GetAndIncTid();

    } else if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
        assert(false);
    }

    return rocksdb::Status::OK();
}

}
