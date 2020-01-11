#pragma once
#include <string>
#include <sstream>
#include "utils.h"

namespace advanced {

template <typename SerializerT>
class FieldT {
public:
    static constexpr const uint8_t TypeValue = 0;
    static constexpr const char* TypeName = "InvalidType";
    using ThisT = FieldT<SerializerT>;

    FieldT(const std::string& name, const std::string& value = "")
        : name_(name), value_(value) {}

    const std::string& Name() const { return name_; }

    template <typename ValT>
    void Serialize(const ValT& val, std::string& serialized) {
        SerializerT::Serialize(val, serialized);
    }

    virtual const char* TName() const {
        return ThisT::TypeName;
    }

    virtual size_t CodeSize() const {
        return value_.size();
    }

    virtual size_t Elements() const {
        return 1;
    }

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

    StringFieldT(const std::string& name, const ValueT& value) : BaseT(name, value) {
    }

    const char* TName() const override{
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

}
