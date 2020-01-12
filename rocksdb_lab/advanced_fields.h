#pragma once
#include <string>
#include <sstream>
#include "utils.h"
#include "iostream"

namespace document {

template <typename SerializerT>
class FieldT {
public:
    static constexpr const uint8_t TypeValue = 0;
    static constexpr const char* TypeName = "InvalidType";
    using ThisT = FieldT<SerializerT>;
    using SerializerType = SerializerT;

    FieldT(const std::string& name, const std::string& value = "")
        : name_(name), value_(value) {}

    const std::string& Name() const { return name_; }

    virtual const char* TName() const {
        return ThisT::TypeName;
    }

    virtual const uint8_t TVale() const { return ThisT::TypeValue; }

    virtual size_t CodeSize() const {
        return value_.size();
    }

    virtual size_t Elements() const {
        return 1;
    }

    bool Serialize(std::string& serialized, bool has_val = false) const {
        uint8_t tval = TVale();
        SerializerT::Serialize(tval, serialized);

        auto size = (uint8_t)name_.size();
        SerializerT::Serialize(size, serialized);
        serialized.append(name_);

        if (!has_val) return true;

        return true;
        // TODO: Handle has_val scenario
    }

    static std::shared_ptr<FieldT<SerializerT>> Deserialize(const rocksdb::Slice& source, size_t& consumed);

    virtual std::string ToPrintableString() const {
        std::stringstream ss;

        ss << "<" << this->TName() << " " << name_ << ":" << value_ << ">";
        return std::move(ss.str());
    }

protected:

    std::string name_;
    std::string value_;
};

struct StringTraits {
    using ValueT = std::string;
    static constexpr const uint8_t TypeValue = 1;
    static constexpr const char* TypeName = "string";
};

template <typename SerializerT, typename TypeTrait>
class StringFieldT : public FieldT<Serializer> {
public:
    static constexpr const uint8_t TypeValue = TypeTrait::TypeValue;
    static constexpr const char* TypeName = TypeTrait::TypeName;
    using BaseT = FieldT<SerializerT>;
    using ThisT = StringFieldT<SerializerT, TypeTrait>;
    using ValueT = typename TypeTrait::ValueT;

    StringFieldT(const std::string& name) : BaseT(name) {
    }

    StringFieldT(const std::string& name, const ValueT& value) : BaseT(name, value) {
    }

    const uint8_t TVale() const override { return ThisT::TypeValue; }

    const char* TName() const override {
        return ThisT::TypeName;
    }
};

using DefaultSerializerT = Serializer;
using Field = FieldT<DefaultSerializerT>;
using StringField = StringFieldT<DefaultSerializerT, StringTraits>;

template <typename SerializerT, typename TypeTrait>
class NumericFieldT : public FieldT<SerializerT> {
public:
    static constexpr const uint8_t TypeValue = TypeTrait::TypeValue;
    static constexpr const char* TypeName = TypeTrait::TypeName;
    using BaseT = FieldT<SerializerT>;
    using ThisT = NumericFieldT<SerializerT, TypeTrait>;
    using ValueT = typename TypeTrait::ValueT;

    NumericFieldT(const std::string& name) : BaseT(name) {
    }

    NumericFieldT(const std::string& name, const ValueT& value) : BaseT(name) {
        SerializerT::Serialize(value, BaseT::value_);
    }

    size_t CodeSize() const override{
        return sizeof(ValueT);
    }

    std::string ToPrintableString() const override {
        std::stringstream ss;
        ValueT v;
        SerializerT::Deserialize(BaseT::value_, v);
        ss << "<" << this->TName() << " " << BaseT::name_ << ":" << v << ">";
        return std::move(ss.str());
    }

    const uint8_t TVale() const override { return ThisT::TypeValue; }

    const char* TName() const override{
        return ThisT::TypeName;
    }
};

struct FloatTraits {
    using ValueT = float;
    static constexpr const uint8_t TypeValue = 2;
    static constexpr const char* TypeName = "float";
};

struct LongTraits {
    using ValueT = long;
    static constexpr const uint8_t TypeValue = 3;
    static constexpr const char* TypeName = "long";
};

struct IntTraits {
    using ValueT = int;
    static constexpr const uint8_t TypeValue = 4;
    static constexpr const char* TypeName = "int";
};

using FloatField = NumericFieldT<DefaultSerializerT, FloatTraits>;
using LongField = NumericFieldT<DefaultSerializerT, LongTraits>;
using IntField = NumericFieldT<DefaultSerializerT, IntTraits>;

template <typename SerializerT>
std::shared_ptr<FieldT<SerializerT>> FieldT<SerializerT>::Deserialize(const rocksdb::Slice& source, size_t& consumed) {
    uint8_t field_type, name_size;
    std::string field_name;
    SerializerT::Deserialize(source.data(), field_type);
    SerializerT::Deserialize(source.data() + sizeof(field_type), name_size);
    field_name.assign(source.data() + sizeof(uint8_t) + sizeof(uint8_t), name_size);
    consumed = sizeof(uint8_t) + sizeof(uint8_t) + name_size;
    if (field_type == FloatTraits::TypeValue) {
        return std::make_shared<NumericFieldT<SerializerT, FloatTraits>>(field_name);
    } else if (field_type == LongTraits::TypeValue) {
        return std::make_shared<NumericFieldT<SerializerT, LongTraits>>(field_name);
    } else if (field_type == IntTraits::TypeValue) {
        return std::make_shared<NumericFieldT<SerializerT, IntTraits>>(field_name);
    } else if (field_type == StringTraits::TypeValue) {
        return std::make_shared<StringFieldT<SerializerT, StringTraits>>(field_name);
    }


    std::cerr << "Cannot serialize field str \"" << source.ToString() << "\"" << std::endl;
    return nullptr;
}

}
