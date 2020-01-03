#include "doc.h"
#include <sstream>
#include <assert.h>

const char* DocSchema::PrimaryKeyName = "_id";
const int DocSchema::LongFieldIdx = 0;
const int DocSchema::StringFieldIdx = 1;
const int DocSchema::FloatFieldIdx = 2;
const int DocSchema::PrimaryKeyIdx = 0;
const int DocSchema::FloatVectorFieldIdx = 3;


DocSchema::DocSchema(const PrimaryKeyT& pk) {
    assert(pk.Name() == PrimaryKeyName);
    AddLongField(pk);
}

DocSchema::DocSchema(PrimaryKeyT&& pk) {
    assert(pk.Name() == PrimaryKeyName);
    AddLongField(std::move(pk));
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

DocSchema& DocSchema::AddLongFieldValue(const std::string& name, long value) {
    LongField field(name);
    field.SetValue(value);
    field.Build();
    return AddLongField(std::move(field));
}

DocSchema& DocSchema::AddFloatFieldValue(const std::string& name, float value) {
    FloatField field(name);
    field.SetValue(value);
    field.Build();
    return AddFloatField(std::move(field));
}

DocSchema& DocSchema::AddStringFieldValue(const std::string& name, const std::string& value) {
    StringField field(name);
    field.SetValue(value);
    field.Build();
    return AddStringField(std::move(field));
}

DocSchema& DocSchema::AddFloatVectorFieldValue(const std::string& name, const std::vector<float>& value) {
    FloatVectorField field(name);
    field.SetValue(value);
    field.Build();
    return AddFloatVectorField(std::move(field));
}

DocSchema& DocSchema::AddLongField(LongField&& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = long_fields_.size();
    fields_schema_[field.Name()] = {LongFieldIdx, offset};
    long_fields_.push_back(std::move(field));
    return *this;
}

DocSchema& DocSchema::AddFloatField(FloatField&& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = float_fields_.size();
    fields_schema_[field.Name()] = {FloatFieldIdx, offset};
    float_fields_.push_back(std::move(field));
    return *this;
}

DocSchema& DocSchema::AddStringField(StringField&& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = string_fields_.size();
    fields_schema_[field.Name()] = {StringFieldIdx, offset};
    string_fields_.push_back(std::move(field));
    return *this;
}

DocSchema& DocSchema::AddFloatVectorField(FloatVectorField&& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = float_vector_fields_.size();
    fields_schema_[field.Name()] = {FloatVectorFieldIdx, offset};
    float_vector_fields_.push_back(std::move(field));
    return *this;
}

template <typename T>
bool DocSchema::PreAddCheck(const T& field) {
    if (HasBuilt()) {
        std::cerr << "Warn: doc schema has already built" << std::endl;
        return false;
    }
    auto& name = field.Name();
    auto it = fields_schema_.find(name);
    if (it != fields_schema_.end()) {
        std::cerr << "Warn: " << name << " has already existed. Skip this add" << std::endl;
        return false;
    }
    return true;
}

DocSchema& DocSchema::AddLongField(const LongField& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = long_fields_.size();
    long_fields_.push_back(field);
    fields_schema_[field.Name()] = {LongFieldIdx, offset};
    return *this;
}

DocSchema& DocSchema::AddFloatField(const FloatField& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = float_fields_.size();
    float_fields_.push_back(field);
    fields_schema_[field.Name()] = {FloatFieldIdx, offset};
    return *this;
}

DocSchema& DocSchema::AddStringField(const StringField& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = string_fields_.size();
    string_fields_.push_back(field);
    fields_schema_[field.Name()] = {StringFieldIdx, offset};
    return *this;
}

DocSchema& DocSchema::AddFloatVectorField(const FloatVectorField& field) {
    auto valid = PreAddCheck(field);
    if (!valid) return *this;

    size_t offset = float_vector_fields_.size();
    float_vector_fields_.push_back(field);
    fields_schema_[field.Name()] = {FloatVectorFieldIdx, offset};
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
        } else if (idx == FloatVectorFieldIdx) {
            ss << "FloatVectorField ";
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
: DocSchema(std::move(pk)), schema_(schema)
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

DocSchema& Doc::AddLongField(const LongField& field) {
    auto& name = field.Name();
    if (!field.HasBuilt()) {
        std::cerr << "Error: field \'" << name << "\' is building" << std::endl;
        assert(false);
    }
    auto it_schema = schema_->fields_schema_.find(name);
    if (it_schema == schema_->fields_schema_.end()) {
        std::cerr << "Error: Cannot add long field \'" << name << "\'" << std::endl;
        assert(false);
    }
    return BaseT::AddLongField(field);
}

DocSchema& Doc::AddFloatField(const FloatField& field) {
    auto& name = field.Name();
    if (!field.HasBuilt()) {
        std::cerr << "Error: field \'" << name << "\' is building" << std::endl;
        assert(false);
    }
    auto it_schema = schema_->fields_schema_.find(name);
    if (it_schema == schema_->fields_schema_.end()) {
        std::cerr << "Error: Cannot add float field \'" << name << "\'" << std::endl;
        assert(false);
    }
    return BaseT::AddFloatField(field);
}

DocSchema& Doc::AddStringField(const StringField& field) {
    auto& name = field.Name();
    if (!field.HasBuilt()) {
        std::cerr << "Error: field \'" << name << "\' is building" << std::endl;
        assert(false);
    }
    auto it_schema = schema_->fields_schema_.find(name);
    if (it_schema == schema_->fields_schema_.end()) {
        std::cerr << "Error: Cannot add string field \'" << name << "\'" << std::endl;
        assert(false);
    }
    return BaseT::AddStringField(field);
}

DocSchema& Doc::AddFloatVectorField(const FloatVectorField& field) {
    auto& name = field.Name();
    if (!field.HasBuilt()) {
        std::cerr << "Error: field \'" << name << "\' is building" << std::endl;
        assert(false);
    }
    auto it_schema = schema_->fields_schema_.find(name);
    if (it_schema == schema_->fields_schema_.end()) {
        std::cerr << "Error: Cannot add string field \'" << name << "\'" << std::endl;
        assert(false);
    }
    return BaseT::AddFloatVectorField(field);
}
