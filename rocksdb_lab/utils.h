#pragma once
#include <algorithm>
#include <rocksdb/db.h>
#include <memory>

template <typename T>
void SwapEndian(T &val, typename std::enable_if<std::is_arithmetic<T>::value, std::nullptr_t>::type = nullptr) {
    union U {
        T val;
        std::array<std::uint8_t, sizeof(T)> raw;
    } src, dst;

    src.val = val;
    std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
    val = dst.val;
}

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

        Serialize(field_type_value, data);
        Serialize(field_name_size, data);
        data.append((char*)v.Name().data(), field_name_size);
        return rocksdb::Status::OK();
    }

    template <typename T>
    static void Serialize(const T& v, std::string& data) {
        auto to_serialized = v;
        SwapEndian(to_serialized);
        data.append((char*)&to_serialized, sizeof(T));
    }

    static void Serialize(const rocksdb::Slice& v, std::string& data) {
        data.append(v.data(), v.size());
    }

    static void Serialize(const std::string& v, std::string& data) {
        Serialize(rocksdb::Slice(v), data);
    }

    template <typename NumT>
    static void Deserialize(const rocksdb::Slice& strv, NumT& numv) {
        numv = *(NumT*)(strv.data());
        SwapEndian(numv);
    }

    static void Deserialize(const rocksdb::Slice& strv, std::string& v) {
        v.append(strv.data(), strv.size());
    }

    static rocksdb::Status DeserializeFieldMeta(const rocksdb::Slice& data, uint8_t& type, std::string& name) {
        type = *data.data();
        uint8_t size = *(data.data() + 1);
        name.assign(data.data()+2, size);
        /* std::cout << __func__ << " name=" << name << " type=" << (int)type << std::endl; */
        return rocksdb::Status::OK();
    }

    /* static rocksdb::Status SerializeDoc(const Doc& doc, std::map<uint8_t, std::string>& data) { */
    /*     data = std::move(doc.Serialize()); */
    /*     return rocksdb::Status::OK(); */
    /* } */
};
