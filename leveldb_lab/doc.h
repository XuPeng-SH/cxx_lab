#pragma once

#include "fields.h"
#include <vector>
#include <map>
#include <type_traits>


class Doc;
class DocSchema {
public:
    using PrimaryKeyT = LongField;
    static const char* PrimaryKeyName;

    DocSchema();
    DocSchema& AddLongField(const std::string& name, const LongField& field);
    DocSchema& AddFloatField(const std::string& name, const FloatField& field);
    DocSchema& AddStringField(const std::string& name, const StringField& field);

    std::string Dump() const;
    DocSchema& Build();

    bool HasBuilt() const { return fixed_; }

private:
    friend class Doc;
    static const int LongFieldIdx;
    static const int StringFieldIdx;
    static const int FloatFieldIdx;
    static const int PrimaryKeyIdx;

    std::map<std::string, std::pair<int, size_t>> fields_schema_;
    std::vector<LongField> long_fields_;
    std::vector<FloatField> float_fields_;
    std::vector<StringField> string_fields_;
    bool fixed_ = false;
};

class Doc {
public:
    using SchemaT = DocSchema;
    using PrimaryKeyT = SchemaT::PrimaryKeyT;

    Doc(const PrimaryKeyT& pk);

    const PrimaryKeyT& GetPK() const { return long_fields_[SchemaT::PrimaryKeyIdx]; }

    /* Doc& AddLongField(const LongField& field); */

private:
    Doc& AddPkField(const PrimaryKeyT& pk);

    std::vector<LongField> long_fields_;
    std::vector<StringField> string_fields_;
    std::vector<FloatField> float_fields_;
    std::map<std::string, std::pair<int, size_t>> name_fields_map_;
};


/* Doc& Doc::AddLongField(const std::string& field_name, const LongField& field) { */
/*     auto it = name_fields_map_.find(field_name); */
/*     if (it != name_fields_map_.end()) { */
/*         auto& pos = it->second; */
/*         auto& idx = std::get<0>(pos); */
/*         auto& offset = std::get<1>(pos); */
/*         if (idx == LongFieldIdx) { */
/*         } else if (idx == FloatField) { */
/*             float_fields_[offset] = field; */
/*         } else if (idx == StringFieldIdx) { */
/*             string_fields_[offset] = field; */
/*         } */
/*         return *this; */
/*     } */

/*     if (idx == LongFieldIdx) { */
/*         long_fields_[offset] = field; */
/*     } else if (idx == FloatField) { */
/*         float_fields_[offset] = field; */
/*     } else if (idx == StringFieldIdx) { */
/*         string_fields_[offset] = field; */
/*     } */
/*     return *this; */
/* } */
