#pragma once

#include "fields.h"
#include <vector>

class Doc {
public:
    using PrimaryKeyT = LongField;
    Doc(const PrimaryKeyT& pk);

    const PrimaryKeyT& GetPK() const { return long_fields_[0]; }

private:
    std::vector<LongField> long_fields_;
    std::vector<StringField> string_fields_;
    std::vector<FloatField> float_fields_;
};

Doc::Doc(const Doc::PrimaryKeyT& pk)
: long_fields_({pk})
{

}
