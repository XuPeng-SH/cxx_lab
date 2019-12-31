#pragma once

#include <string>
#include <vector>
#include <map>



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


class StringMixin {
public:
    static constexpr size_t MAX_LENGTH = 1024;

    int MaxLength() const { return max_length_; }
    int MinLength() const { return min_length_; }

    bool SetMaxLength(int length) {
        if (length < 0 || length < min_length_ || length > MAX_LENGTH) {
            /* std::cerr << __func__ << " Invalid Length: " << length << endl; */
            return false;
        }
        max_length_ = length;
        return true;
    }

    bool SetMinLength(int length) {
        if (length < 0 || length > max_length_) {
            /* std::cerr << __func__ << " Invalid Length: " << length << endl; */
            return false;
        }
        min_length_ = length;
        return true;
    }

    bool Validate(const std::string& value) const {
        return value.size() >= min_length_ && value.size() <= max_length_;
    }

protected:
    int max_length_ = MAX_LENGTH;
    int min_length_ = 0;

};

template <typename T>
class TypedField : public Raw {
public:
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

using BooleanField = TypedField<bool>;
using IntField = TypedField<int>;
using FloatField = TypedField<float>;
using DoubleField = TypedField<double>;

class StringField : public StringMixin, public TypedField<std::string> {
public:
    using MixinT = StringMixin;
    bool Validate() const override {
        return MixinT::Validate(value_);
    }
};
