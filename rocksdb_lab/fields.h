#pragma once

#include <string>
#include <vector>
#include <map>
#include <limits>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <typeinfo>
#include <type_traits>


template <typename T>
struct TypeWrapper {};


template <typename T>
class TypedField {
public:
    using ValueT = T;
    using ThisT = TypedField<T>;
    using TypeInfoT = TypeWrapper<ValueT>;

    TypedField() = delete;
    TypedField(const std::string& name) : name_(name) {}

    TypedField(TypedField&& other) {
        name_ = std::move(other.name_);
        fixed_ = other.fixed_;
        initialized_ = other.initialized_;
        readonly_ = other.readonly_;
        required_ = other.required_;
        unique_ = other.unique_;
        value_ = std::move(other.value_);
    }

    TypedField(const TypedField& other) {
        name_ = other.name_;
        fixed_ = other.fixed_;
        initialized_ = other.initialized_;
        readonly_ = other.readonly_;
        required_ = other.required_;
        unique_ = other.unique_;
        value_ = other.value_;
    }

    virtual ~TypedField() {};

    TypedField& SetReadonly(bool ro) {readonly_ = ro;}
    TypedField& SetRequired(bool req) {required_ = req;}
    TypedField& SetUnique(bool uni) {unique_ = uni;}

    bool HasValue() const { return initialized_; }
    bool IsReadonly() const { return readonly_; }
    bool IsRequried() const { return required_; }
    bool IsUnique() const { return unique_; }
    bool HasBuilt() const { return fixed_; }

    const std::string& Name() const { return name_; }

    static const char* FieldTypeName() {
        return TypeInfoT::name;
    }

    static const uint8_t FieldTypeValue() {
        return TypeInfoT::value;
    }

    virtual bool Validate() const {return true;};
    virtual ThisT& Build() {
        if (!HasValue()) {
            std::cerr << "Error: Build field" << std::endl;
            assert(false);
        }
        fixed_ = true;
        return *this;
    }

    bool SetValue(const ValueT& val) {
        if (HasBuilt() && IsReadonly()) {
            std::cerr << "Error: Readonly field has been built but set value again!" << std::endl;
            return false;
        }
        initialized_ = true;
        value_ = val;
        return true;
    }

    const ValueT& GetValue() const {return value_;}

    virtual std::string DumpValue() const {
        return "";
    }

    virtual std::map<std::string, std::string> ToMap() const {
        std::map<std::string, std::string> map_result;
        map_result["RO"] = IsReadonly() ? "RO" : "";
        map_result["UNIQUE"] = IsUnique() ? "UNIQUE" : "";
        map_result["REQURIED"] = IsRequried() ? "REQURIED" : "";
        map_result["VALUE"] = DumpValue();

        return map_result;
    }

    virtual std::string Dump() const {
        std::stringstream ss;
        auto attrs_map = ToMap();
        ss << "<" << FieldTypeName() <<  "field \'" << name_ << "\'>\n";
        for (auto& kv: attrs_map) {
            if (kv.second == "") continue;
            ss << "  "  << kv.first << ": " << kv.second << "\n";
        }
        return ss.str();
    }

protected:
    std::string name_;
    bool fixed_ = false;
    bool initialized_ = false;
    bool readonly_ = false;
    bool required_ = false;
    bool unique_ = false;
    ValueT value_;
};


template <typename FieldT>
class LengthMixin {
public:
    using ValueT = typename FieldT::ValueT;
    static constexpr size_t MAX_LENGTH = 1024;

    int MaxLength() const { return max_length_; }
    int MinLength() const { return min_length_; }

    bool SetMaxLength(int length) {
        if (fixed_) {
            std::cerr << "Error: field has been built but set max length again!" << std::endl;
            return false;
        }
        if (length < 0 || length < min_length_ || length > MAX_LENGTH) {
            std::cerr << "Error: invalid max length!" << std::endl;
            return false;
        }
        max_length_ = length;
        return true;
    }

    bool SetMinLength(int length) {
        if (fixed_) {
            std::cerr << "Error: field has been built but set min length again!" << std::endl;
            return false;
        }
        if (length < 0 || length > max_length_) {
            std::cerr << "Error: invalid min length!" << std::endl;
            return false;
        }
        min_length_ = length;
        return true;
    }

    bool Validate(const ValueT& value) const {
        return value.size() >= min_length_ && value.size() <= max_length_;
    }

    void Build() {
        fixed_ = true;
    }

protected:
    bool fixed_ = false;
    int max_length_ = MAX_LENGTH;
    int min_length_ = 0;
};

template <typename FieldT>
class VectorLengthMixin : public LengthMixin<FieldT> {
public:
    using ValueT = typename FieldT::ValueT;
    static constexpr size_t MAX_LENGTH = std::numeric_limits<size_t>::max();
};

template <typename FieldT>
class MinMaxMixin {
public:
    using ValueT = typename FieldT::ValueT;

    const ValueT& MaxLimit() const { return max_v_; }
    const ValueT& MinLimit() const { return min_v_; }

    bool SetMinLimit(const ValueT& value) {
        if (fixed_) {
            std::cerr << "Error: field has been built but set min limit again!" << std::endl;
            return false;
        }
        if (value > max_v_) {
            std::cerr << "Error: invalid min value!" << std::endl;
            return false;
        }
        min_v_ = value;
        return true;
    }

    bool SetMaxLimit(const ValueT& value) {
        if (fixed_) {
            std::cerr << "Error: field has been built but set max limit again!" << std::endl;
            return false;
        }
        if (value < min_v_) {
            std::cerr << "Error: invalid max value!" << std::endl;
            return false;
        }
        max_v_ = value;
        return true;
    }

    bool Validate(const ValueT& value) const {
        return value >= min_v_ && value <= max_v_;
    }

    void Build() {
        fixed_ = true;
    }

protected:
    bool fixed_ = false;
    ValueT max_v_ = std::numeric_limits<ValueT>::max();
    ValueT min_v_ = std::numeric_limits<ValueT>::min();
};

template <template<class> class Mixin, typename ValueT>
class WithMixinTypedField : public Mixin<TypedField<ValueT>>, public TypedField<ValueT> {
public:
    using BaseT = TypedField<ValueT>;
    using MixinT = Mixin<BaseT>;
    using ThisT = WithMixinTypedField;

    WithMixinTypedField(const std::string& name) : BaseT::ThisT(name) {}

    bool Validate() const override {
        return MixinT::Validate(BaseT::value_);
    }

    ThisT& Build() override {
        BaseT::Build();
        MixinT::Build();
        return *this;
    }
};

template <typename T>
class NumericField : public WithMixinTypedField<MinMaxMixin, T> {
public:
    using BaseT = WithMixinTypedField<MinMaxMixin, T>;
    using ValueT = T;

    NumericField(const std::string& name) : BaseT::ThisT(name) {}

    std::string DumpValue() const override{
        std::stringstream ss;
        ss << BaseT::GetValue();
        return ss.str();
    }
};

template <>
struct TypeWrapper<long> {
    static constexpr const char* name = "LongField";
    static constexpr const uint8_t value = 1;
};
using LongField = NumericField<long>;

template <>
struct TypeWrapper<float> {
    static constexpr const char* name = "FloatField";
    static constexpr const uint8_t value = 2;
};
using FloatField = NumericField<float>;

template <>
struct TypeWrapper<std::string> {
    static constexpr const char* name = "StringField";
    static constexpr const uint8_t value = 3;
};

class StringField : public WithMixinTypedField<LengthMixin, std::string> {
public:
    using BaseT = WithMixinTypedField<LengthMixin, std::string>;

    StringField(const std::string& name) : BaseT::ThisT(name) {}

    std::string DumpValue() const override{
        return BaseT::GetValue();
    }
};

template <typename ElementT>
class VectorField : public WithMixinTypedField<VectorLengthMixin, std::vector<ElementT>> {
public:
    using BaseT = WithMixinTypedField<VectorLengthMixin, std::vector<ElementT>>;

    VectorField(const std::string& name) : BaseT::ThisT(name) {}
    std::string DumpValue() const override{
        std::stringstream ss;
        for (auto& v : BaseT::value_) {
            ss << v << " ";
        }
        return ss.str();
    }
};

template <>
struct TypeWrapper<std::vector<float>> {
    static constexpr const char* name = "FloatVectorField";
    static constexpr const uint8_t value = 4;
};
using FloatVectorField = VectorField<float>;

template <>
struct TypeWrapper<bool> {
    static constexpr const char* name = "BooleanField";
    static constexpr const uint8_t value = 5;
};
using BooleanField = TypedField<bool>;
