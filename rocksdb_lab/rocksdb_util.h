#pragma once

#include <rocksdb/db.h>
#include <memory>
#include <iostream>
#include "doc.h"

namespace db {

static constexpr const size_t DBTableSegmentSize = 50000; // TODO: Support Customized size

static constexpr const size_t PrefixSize = 3;

// (K,V) = (TSK, 1)        ==> The next Table ID to be allocated is 1
// (K,V) = (TSK, 28)       ==> The next Table ID to be allocated is 28
static const std::string DBTableSequenceKey = "TSK";

// (K,V) = (T:t0, 0)        ==> Table Name 't0' is assigned with Table ID 0
// (K,V) = (T:new_t, 1)     ==> Table Name 'new_t' is assigned with Table ID 1
static const std::string DBTablePrefix = "TTP";

// (K,V) = (TS:0, 0)     ==> Table ID 0 current used segment is 0
// (K,V) = (TS:2, 5)     ==> Table ID 2 current used segment is 5
// [Key]$Prefix:$tid  [Val]$sid
static const std::string DBTableCurrentSegmentPrefix = "TSP";

// [Key]$Prefix:$tid:$sid  [Val]$id
// (K,V) = (TSS:0:0, 0)    ==> Table/Segment ID 0/0 next id is 0
static const std::string DBTableSegmentNextIDPrefix = "TSS";

// [Key]ID:$tid:$uid [Val]$sid$id
// (K,V) = (ID:0:1837493493434, 0'0)   ==> Table ID 0 UID=1837493493434 Points to $sid0 $id=0
// TODO: Use CF to redesign Key
static const std::string DBTableUidIdMappingPrefix = "TID";

// [Key]TM:$tid [Val]$mappings
// (K,V) = (TM:0, [{"name": STRING}, [{"age": UINT8}]]) ==> Table 0 has 2 fields of types STRING and UINT8
static const std::string DBTableMappingPrefix = "TMP";

// [Key]$Prefix:$tid:$fid$fval$sid$id   [Val]None
static const std::string DBTableFieldIndexPrefix = "TFI";

// [Key]$Prefix:$tid:$sid$id$fid    [val]$fval
static const std::string DBTableFieldValuePrefix = "TFV";

const std::shared_ptr<rocksdb::Options>& DefaultOpenOptions();
const std::shared_ptr<rocksdb::WriteOptions>& DefaultDBWriteOptions();

class MyComparator : public rocksdb::Comparator {
public:
    const char* Name() const override { return "db.MyComparator"; }
    int Compare(const rocksdb::Slice& a, const rocksdb::Slice& b) const override;

    void FindShortSuccessor(std::string* key) const override {/*std::cout << "YYYYY" << std::endl;*/}
    void FindShortestSeparator(std::string* start, const rocksdb::Slice& limit) const override {
    }
};

namespace demo {
    void just_check_cmp(std::shared_ptr<rocksdb::DB> db);
    void check_str_to_uint64();
    /* void read_all(std::shared_ptr<rocksdb::DB> db, rocksdb::ReadOptions* options, bool do_print = true); */
    void write_batch_demo(std::shared_ptr<rocksdb::DB>);
    void mock_uid_id_mapping(std::shared_ptr<rocksdb::DB> db, int num = 0);
}

}
