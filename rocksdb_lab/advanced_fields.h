#pragma once
#include <string>
#include <sstream>
#include "utils.h"

namespace advanced {

template <typename SerializerT>
class FieldT {
public:
    static constexpr const uint8_t Type = 0;
    static constexpr const char* TypeName = "InvalidType";
    using ThisT = FieldT<SerializerT>;

    FieldT(const std::string& name, const std::string& value = "")
        : name_(name), value_(value) {}
    size_t Size() const { return value_.size(); }
    const std::string& Name() const { return name_; }

    template <typename ValT>
    void Serialize(const ValT& val, std::string& serialized) {
        SerializerT::Serialize(val, serialized);
    }

    virtual const char* TName() const {
        return ThisT::TypeName;
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

template <typename SerializerT>
class StringFieldT : public FieldT<Serializer> {
public:
    static constexpr const uint8_t Type = 1;
    static constexpr const char* TypeName = "String";
    using BaseT = FieldT<SerializerT>;
    using ThisT = StringFieldT<SerializerT>;

    StringFieldT(const std::string& name, const std::string& value) : BaseT(name, value) {
    }

    const char* TName() const override{
        return ThisT::TypeName;
    }
};

using StringField = StringFieldT<Serializer>;

template <typename SerializerT>
class FloatFieldT : public FieldT<SerializerT> {
public:
    static constexpr const uint8_t Type = 2;
    static constexpr const char* TypeName = "Float";
    using BaseT = FieldT<SerializerT>;
    using ThisT = FloatFieldT<SerializerT>;

    FloatFieldT(const std::string& name, float value) : BaseT(name) {
        SerializerT::Serialize(value, BaseT::value_);
    }

    std::string ToPrintableString() const override {
        std::stringstream ss;
        float v;
        SerializerT::Deserialize(BaseT::value_, v);
        ss << "<" << this->TName() << " " << BaseT::name_ << ":" << v << ">";
        return std::move(ss.str());
    }

    const char* TName() const override{
        return ThisT::TypeName;
    }
};

using FloatField = FloatFieldT<Serializer>;

}
