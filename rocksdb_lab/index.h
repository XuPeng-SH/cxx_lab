#pragma once
#include <string>
#include <rocksdb/db.h>
#include <memory>
#include <map>
#include <tuple>


namespace faiss {
    class Index;
}

class VectorIndexImpl;
using IndexPtr = std::shared_ptr<VectorIndexImpl>;

using IndexKeyT = std::tuple<uint64_t, uint64_t>;

class VectorIndexImpl;
struct IndexHub {
     using VectorMapT = std::map<IndexKeyT, IndexPtr>;
     /* rocksdb::Status Recover(const rocksdb::DB& db); */
     bool SetIndex(uint64_t tid, uint64_t sid, IndexPtr index);

     IndexPtr GetIndex(uint64_t tid, uint64_t sid);

protected:
     VectorMapT indice_;
};

struct VectorIndexImpl {
    /* virtual rocksdb::Status Recover(const rocksdb::DB& db); */
    virtual ~VectorIndexImpl() {}
};

#ifdef WITH_FAISS

struct VectorFaissIndexImpl {
    /* rocksdb::Status Recover(const rocksdb::DB& db) override; */
protected:
    std::shared_ptr<faiss::Index> index_;
};

#endif
