#pragma once

#include <rocksdb/db.h>
#include <memory>

namespace db {

typedef enum {
    INT8 = 0,
    UINT8,
    INT32,
    UINT32,
    INT64,
    UINT64,

    FLOAT=50,

    STRING=100,

} FieldTypeEnum;

static constexpr const size_t DBTableSegmentSize = 500000; // TODO: Support Customized size

// (K,V) = (TSK, 1)        ==> The next Table ID to be allocated is 1
// (K,V) = (TSK, 28)       ==> The next Table ID to be allocated is 28
static const std::string DBTableSequenceKey = "TSK";

// (K,V) = (T:t0, 0)        ==> Table Name 't0' is assigned with Table ID 0
// (K,V) = (T:new_t, 1)     ==> Table Name 'new_t' is assigned with Table ID 1
static const std::string DBTablePrefix = "T:";

// (K,V) = (TS:0, 0)     ==> Table ID 0 current used segment is 0
// (K,V) = (TS:2, 5)     ==> Table ID 2 current used segment is 5
static const std::string DBTableCurrentSegmentPrefix = "TS:";

// [Key]TSS:$tid:$sid  [Val]$id
// (K,V) = (TSS:0:0, 0)    ==> Table/Segment ID 0/0 next id is 0
static const std::string DBTableSegmentNextIDPrefix = "TSS:";

// [Key]ID:$tid:$uid [Val]$sid$id
// (K,V) = (ID:0:1837493493434, 0'0)   ==> Table ID 0 UID=1837493493434 Points to $sid0 $id=0
// TODO: Use CF to redesign Key
static const std::string DBTableUidIdMappingPrefix = "ID:";

// [Key]TM:$tid [Val]$mappings
// (K,V) = (TM:0, [{"name": STRING}, [{"age": UINT8}]]) ==> Table 0 has 2 fields of types STRING and UINT8
static const std::string DBTableMappingPrefix = "TM:";

// [Key]TF:$tid:$sid:$id:$fid [Val]$fval
// (K,V) = (ID:0:0:20:1, 20)   ==> Table/Segment/ID 0/0/20 filed id 1 store value 20
static const std::string DBTableFieldValuePrefix = "TF:";

const std::shared_ptr<rocksdb::Options>& DefaultOpenOptions();
const std::shared_ptr<rocksdb::WriteOptions>& DefaultDBWriteOptions();

namespace demo {
    void read_all(std::shared_ptr<rocksdb::DB> db, bool do_print = true);
    void write_batch_demo(std::shared_ptr<rocksdb::DB>);
}

}
