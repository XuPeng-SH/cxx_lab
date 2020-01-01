#include "doc.h"
#include <sstream>

const char* DocSchema::PrimaryKeyName = "_id";
const int DocSchema::LongFieldIdx = 0;
const int DocSchema::StringFieldIdx = 1;
const int DocSchema::FloatFieldIdx = 2;
const int DocSchema::PrimaryKeyIdx = 0;


DocSchema::DocSchema() {
    AddLongField(PrimaryKeyName, LongField());
}

DocSchema& DocSchema::Build() {
    fixed_ = true;
}

DocSchema& DocSchema::AddLongField(const std::string& name, const LongField& field) {
    if (HasBuilt()) {
        std::cerr << "Warn: doc schema has already built" << std::endl;
        return *this;
    }
    auto it = fields_schema_.find(name);
    if (it != fields_schema_.end()) {
        std::cerr << "Warn: " << name << " has already existed. Skip this add" << std::endl;
        return *this;
    }

    size_t offset = long_fields_.size();
    long_fields_.push_back(field);
    fields_schema_[name] = {LongFieldIdx, offset};
    return *this;
}

DocSchema& DocSchema::AddFloatField(const std::string& name, const FloatField& field) {
    if (HasBuilt()) {
        std::cerr << "Warn: doc schema has already built" << std::endl;
        return *this;
    }
    auto it = fields_schema_.find(name);
    if (it != fields_schema_.end()) {
        std::cerr << "Warn: " << name << " has already existed. Skip this add" << std::endl;
        return *this;
    }

    size_t offset = float_fields_.size();
    float_fields_.push_back(field);
    fields_schema_[name] = {FloatFieldIdx, offset};
    return *this;
}

DocSchema& DocSchema::AddStringField(const std::string& name, const StringField& field) {
    if (HasBuilt()) {
        std::cerr << "Warn: doc schema has already built" << std::endl;
        return *this;
    }
    auto it = fields_schema_.find(name);
    if (it != fields_schema_.end()) {
        std::cerr << "Warn: " << name << " has already existed. Skip this add" << std::endl;
        return *this;
    }

    size_t offset = string_fields_.size();
    string_fields_.push_back(field);
    fields_schema_[name] = {StringFieldIdx, offset};
    return *this;
}

std::string DocSchema::Dump() const {
    std::stringstream ss;
    ss << "DocSchema: " << (HasBuilt()?"[Built]":"[BUILDING]")  << "\n";
    for (auto& kv: fields_schema_) {
        auto& field_name = kv.first;
        auto& idx = std::get<0>(kv.second);
        auto& offset = std::get<1>(kv.second);

        ss << field_name << " ";
        if (idx == LongFieldIdx) {
            ss << "LongField ";
        } else if (idx == FloatFieldIdx) {
            ss << "FloatField ";
        } else if (idx == StringFieldIdx) {
            ss << "StringField ";
        }

        if (idx == LongFieldIdx && offset == 0) {
            ss << "PK ";
        }
        ss << "\n";
    }
    return ss.str();
}

Doc::Doc(const Doc::PrimaryKeyT& pk)
{
    AddPkField(pk);
}

Doc& Doc::AddPkField(const PrimaryKeyT& pk) {
    long_fields_ = {pk};
    auto idx = SchemaT::PrimaryKeyIdx;
    name_fields_map_[SchemaT::PrimaryKeyName] = {idx, 0};
    return *this;
}
