#pragma once

#include <rocksdb/db.h>
#include <memory>
#include <iostream>
#include "doc.h"

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

/* static constexpr const size_t DBTableSegmentSize = 1; // TODO: Support Customized size */
static constexpr const size_t DBTableSegmentSize = 500000; // TODO: Support Customized size

static constexpr const size_t PrefixSize = 3;

// (K,V) = (TSK, 1)        ==> The next Table ID to be allocated is 1
// (K,V) = (TSK, 28)       ==> The next Table ID to be allocated is 28
static const std::string DBTableSequenceKey = "TSK";

// (K,V) = (T:t0, 0)        ==> Table Name 't0' is assigned with Table ID 0
// (K,V) = (T:new_t, 1)     ==> Table Name 'new_t' is assigned with Table ID 1
static const std::string DBTablePrefix = "TTP";

// (K,V) = (TS:0, 0)     ==> Table ID 0 current used segment is 0
// (K,V) = (TS:2, 5)     ==> Table ID 2 current used segment is 5
static const std::string DBTableCurrentSegmentPrefix = "TSP";

// [Key]TSS:$tid:$sid  [Val]$id
// (K,V) = (TSS:0:0, 0)    ==> Table/Segment ID 0/0 next id is 0
static const std::string DBTableSegmentNextIDPrefix = "TSS";

// [Key]ID:$tid:$uid [Val]$sid$id
// (K,V) = (ID:0:1837493493434, 0'0)   ==> Table ID 0 UID=1837493493434 Points to $sid0 $id=0
// TODO: Use CF to redesign Key
static const std::string DBTableUidIdMappingPrefix = "TID";

// [Key]TM:$tid [Val]$mappings
// (K,V) = (TM:0, [{"name": STRING}, [{"age": UINT8}]]) ==> Table 0 has 2 fields of types STRING and UINT8
static const std::string DBTableMappingPrefix = "TMP";

// [Key]TF:$tid:$fid$fval$sid$id [Val]
// (K,V) = (ID:0:0:20:1, 20)   ==> Table/Segment/ID 0/0/20 filed id 1 store value 20
static const std::string DBTableFieldValuePrefix = "TFP";

const std::shared_ptr<rocksdb::Options>& DefaultOpenOptions();
const std::shared_ptr<rocksdb::WriteOptions>& DefaultDBWriteOptions();

class DocSchemaSerializerHandler : public DocSchemaHandler {
public:
    void PreHandle(const DocSchema& schema) override;
    void Handle(const DocSchema& schema, const std::string& field_name, uint8_t field_id,
        int idx, size_t offset) override;
    void PostHandle(const DocSchema& schema) override;
    const std::string& ToString() const;
    std::string&& ToString();

protected:
    std::string serialized_;
};

class Serializer {
public:
    /* using ValueT = typename FieldT::ValueT; */

    template <typename FieldT>
    static rocksdb::Status SerializeFieldMeta(const FieldT& v, std::string& data) {
        uint8_t field_type_value = v.FieldTypeValue();
        /* std::cout << __func__ << " type=" << (int)field_type_value << std::endl; */
        uint8_t field_name_size = (uint8_t)v.Name().size();
        // [$field_type_value][$field_name_size][$field_name]
        // |------uint8_t-----|----uint8_t-----|---n bytes--|
        data.append((char*)&field_type_value, sizeof(field_type_value));
        data.append((char*)&field_name_size, sizeof(field_name_size));
        data.append((char*)v.Name().data(), field_name_size);
        return rocksdb::Status::OK();
    }

    static rocksdb::Status DeserializeFieldMeta(const rocksdb::Slice& data, uint8_t& type, std::string& name) {
        type = *data.data();
        uint8_t size = *(data.data() + 1);
        name.assign(data.data()+2, size);
        /* std::cout << __func__ << " name=" << name << " type=" << (int)type << std::endl; */
        return rocksdb::Status::OK();
    }

    static rocksdb::Status SerializeDoc(const Doc& doc, std::map<uint8_t, std::string>& data) {
        data = std::move(doc.Serialize());
        return rocksdb::Status::OK();
    }

    static rocksdb::Status SerializeDocSchema(const DocSchema& schema, std::string& data) {
        DocSchemaSerializerHandler handler;
        schema.Iterate(&handler);
        data = std::move(handler.ToString());
        return rocksdb::Status::OK();
    }

    static rocksdb::Status DeserializeDocSchema(const rocksdb::Slice& data, DocSchema& schema) {
        uint8_t fields_num = *(uint8_t*)(data.data());
        int offset = 1;
        uint8_t field_id;
        uint8_t field_type;
        uint8_t name_size;
        std::string field_name;
        while(fields_num-- > 0) {
            field_id = *(uint8_t*)(data.data() + offset++);
            field_type = *(uint8_t*)(data.data() + offset++);
            name_size = *(uint8_t*)(data.data() + offset++);

            field_name.assign(data.data()+offset, name_size);
            offset += name_size;
            /* std::cout << "field_id=" << (int)field_id << " field_type=" << (int)field_type; */
            /* std::cout << " field_name=" << field_name << std::endl; */

            // TODO: Store and fetch field parameters
            if (field_type == LongField::FieldTypeValue()) {
                LongField f(field_name);
                schema.AddLongField(std::move(f));
            } else if (field_type == StringField::FieldTypeValue()) {
                StringField f(field_name);
                schema.AddStringField(std::move(f));
            } else if (field_type == FloatField::FieldTypeValue()) {
                FloatField f(field_name);
                schema.AddFloatField(std::move(f));
            }
        }

        schema.Build();
        /* std::cout << schema.Dump() << std::endl; */

        return rocksdb::Status::OK();
    }
};

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
