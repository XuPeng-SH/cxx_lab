#pragma once

#include <string>
#include <vector>
#include <map>
#include <limits>
#include <iostream>
#include <assert.h>


template <typename T>
class TypedField {
public:
    using ValueT = T;
    using ThisT = TypedField<T>;
    virtual ~TypedField() {};

    TypedField& SetReadonly(bool ro) {readonly_ = ro;}
    TypedField& SetRequired(bool req) {required_ = req;}

    bool HasValue() const { return initialized_; }
    bool IsReadonly() const { return readonly_; }
    bool IsRequried() const { return required_; }
    bool HasBuilt() const { return fixed_; }

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

protected:
    bool fixed_ = false;
    bool initialized_ = false;
    bool readonly_ = false;
    bool required_ = false;
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

    bool Validate() const override {
        return MixinT::Validate(BaseT::value_);
    }

    ThisT& Build() override {
        BaseT::Build();
        MixinT::Build();
        return *this;
    }
};

template <typename ValueT>
using NumericField = WithMixinTypedField<MinMaxMixin, ValueT>;

using IntField = NumericField<int>;
using LongField = NumericField<long>;
using FloatField = NumericField<float>;
using DoubleField = NumericField<double>;
using StringField = WithMixinTypedField<LengthMixin, std::string>;
template <typename ElementT>
using VectorField = WithMixinTypedField<VectorLengthMixin, std::vector<ElementT>>;

using FloatVectorField = VectorField<float>;

using BooleanField = TypedField<bool>;

class FieldFactory {
public:
    template <typename ElementT>
    static VectorField<ElementT> BuildVectorField(size_t dimension) {
        VectorField<ElementT> vf;
        vf.SetMaxLength(dimension);
        vf.SetMinLength(dimension);
        return std::move(vf);
    }
};
