#include "doc.h"
#include <sstream>
#include <assert.h>

const char* DocSchema::PrimaryKeyName = "_id";
const int DocSchema::LongFieldIdx = 0;
const int DocSchema::StringFieldIdx = 1;
const int DocSchema::FloatFieldIdx = 2;
const int DocSchema::PrimaryKeyIdx = 0;


DocSchema::DocSchema(const PrimaryKeyT& pk) {
    AddLongField(PrimaryKeyName, pk);
}

DocSchema::DocSchema(PrimaryKeyT&& pk) {
    AddLongField(PrimaryKeyName, pk);
}

DocSchema::DocSchema(DocSchema&& other)
: fields_schema_(std::move(other.fields_schema_)),
  long_fields_(std::move(other.long_fields_)),
  float_fields_(std::move(other.float_fields_)),
  string_fields_(std::move(other.string_fields_)),
  fixed_(other.fixed_)
{
}

bool DocSchema::Build() {
    fixed_ = true;
    return true;
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

Doc::Doc(const PrimaryKeyT& pk, const std::shared_ptr<DocSchema> schema)
: DocSchema(pk), schema_(schema)
{
    assert(pk.HasBuilt());
    assert(schema->HasBuilt());
}

Doc::Doc(PrimaryKeyT&& pk, const std::shared_ptr<DocSchema> schema)
: DocSchema(pk), schema_(schema)
{
    assert(pk.HasBuilt());
    assert(schema->HasBuilt());
}

bool Doc::Build() {
    if (fields_schema_.size() != schema_->fields_schema_.size()) {
        std::cerr << "Error: Cannot build doc due to incomplete fields" << std::endl;
        return false;
    }
    return BaseT::Build();
}

DocSchema& Doc::AddLongField(const std::string& name, const LongField& field) {
    if (!field.HasBuilt()) {
        std::cerr << "Error: field \'" << name << "\' is building" << std::endl;
        assert(false);
    }
    auto it_schema = schema_->fields_schema_.find(name);
    if (it_schema == schema_->fields_schema_.end()) {
        std::cerr << "Error: Cannot add long field \'" << name << "\'" << std::endl;
        assert(false);
    }
    return BaseT::AddLongField(name, field);
}

DocSchema& Doc::AddFloatField(const std::string& name, const FloatField& field) {
    if (!field.HasBuilt()) {
        std::cerr << "Error: field \'" << name << "\' is building" << std::endl;
        assert(false);
    }
    auto it_schema = schema_->fields_schema_.find(name);
    if (it_schema == schema_->fields_schema_.end()) {
        std::cerr << "Error: Cannot add float field \'" << name << "\'" << std::endl;
        assert(false);
    }
    return BaseT::AddFloatField(name, field);
}

DocSchema& Doc::AddStringField(const std::string& name, const StringField& field) {
    if (!field.HasBuilt()) {
        std::cerr << "Error: field \'" << name << "\' is building" << std::endl;
        assert(false);
    }
    auto it_schema = schema_->fields_schema_.find(name);
    if (it_schema == schema_->fields_schema_.end()) {
        std::cerr << "Error: Cannot add string field \'" << name << "\'" << std::endl;
        assert(false);
    }
    return BaseT::AddStringField(name, field);
}