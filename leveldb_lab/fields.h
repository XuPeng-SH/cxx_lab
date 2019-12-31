#pragma once

#include <string>
#include <vector>
#include <map>
#include <limits>



class Raw {
public:
    virtual ~Raw() {};

    Raw& SetReadonly(bool ro) {readonly_ = ro;}
    Raw& SetRequired(bool req) {required_ = req;}

    bool HasValue() const { return initialized_; }
    bool IsReadonly() const { return readonly_; }
    bool IsRequried() const { return required_; }

    virtual bool Validate() const {return true;};

protected:
    bool initialized_ = false;
    bool readonly_ = false;
    bool required_ = false;
};


template <typename FieldT>
class LengthMixin {
public:
    using ValueT = typename FieldT::ValueT;
    static constexpr size_t MAX_LENGTH = 1024;

    int MaxLength() const { return max_length_; }
    int MinLength() const { return min_length_; }

    bool SetMaxLength(int length) {
        if (length < 0 || length < min_length_ || length > MAX_LENGTH) {
            return false;
        }
        max_length_ = length;
        return true;
    }

    bool SetMinLength(int length) {
        if (length < 0 || length > max_length_) {
            return false;
        }
        min_length_ = length;
        return true;
    }

    bool Validate(const ValueT& value) const {
        return value.size() >= min_length_ && value.size() <= max_length_;
    }

protected:
    int max_length_ = MAX_LENGTH;
    int min_length_ = 0;
};


template <typename FieldT>
class MinMaxMixin {
public:
    using ValueT = typename FieldT::ValueT;

    const ValueT& MaxLimit() const { return max_v_; }
    const ValueT& MinLimit() const { return min_v_; }

    bool SetMinLimit(const ValueT& value) {
        if (value > max_v_) {
            return false;
        }
        min_v_ = value;
        return true;
    }

    bool SetMaxLimit(const ValueT& value) {
        if (value < min_v_) {
            return false;
        }
        max_v_ = value;
        return true;
    }

    bool Validate(const ValueT& value) const {
        return value >= min_v_ && value <= max_v_;
    }

protected:
    ValueT max_v_ = std::numeric_limits<ValueT>::max();
    ValueT min_v_ = std::numeric_limits<ValueT>::min();
};

template <typename T>
class TypedField : public Raw {
public:
    using ValueT = T;

    bool SetValue(const T& val) {
        if (HasValue() && IsReadonly()) {
            return false;
        }
        initialized_ = true;
        value_ = val;
        return true;
    }

    const T& GetValue() const {return value_;}

protected:
    T value_;
};

template <template<class> class Mixin, typename ValueT>
class WithMixinTypedField : public Mixin<TypedField<ValueT>>, public TypedField<ValueT> {
public:
    using BaseT = TypedField<ValueT>;
    using MixinT = Mixin<BaseT>;

    bool Validate() const override {
        return MixinT::Validate(BaseT::value_);
    }
};

template <typename ValueT>
using NumericField = WithMixinTypedField<MinMaxMixin, ValueT>;

using IntField = NumericField<int>;
using FloatField = NumericField<float>;
using DoubleField = NumericField<double>;
using StringField = WithMixinTypedField<LengthMixin, std::string>;
using BooleanField = TypedField<bool>;

using BooleanField = TypedField<bool>;
