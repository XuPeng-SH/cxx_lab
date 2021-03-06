#pragma once

#include "fields.h"
#include <vector>
#include <map>
#include <type_traits>
#include <memory>
#include <rocksdb/db.h>

class Doc;
class DocSchemaHandler;
class DocSchemaSerializerHandler;
class DocSchema {
public:
    using PrimaryKeyT = LongField;
    static const char* PrimaryKeyName;
    static const int LongFieldIdx;
    static const int StringFieldIdx;
    static const int FloatFieldIdx;
    static const int PrimaryKeyIdx;
    static const int FloatVectorFieldIdx;

    DocSchema() = default;
    /* DocSchema(const PrimaryKeyT& pk = PrimaryKeyT(PrimaryKeyName)); */
    DocSchema(const PrimaryKeyT& pk);
    DocSchema(PrimaryKeyT&& pk);

    DocSchema(DocSchema&& other);
    DocSchema(const DocSchema& other);

    /* virtual DocSchema& AddPKField(const LongField& pk); */
    virtual DocSchema& AddLongField(const LongField& field);
    virtual DocSchema& AddFloatField(const FloatField& field);
    virtual DocSchema& AddStringField(const StringField& field);
    virtual DocSchema& AddFloatVectorField(const FloatVectorField& field);

    /* virtual DocSchema& AddPKField(LongField&& pk); */
    virtual DocSchema& AddLongField(LongField&& field);
    virtual DocSchema& AddFloatField(FloatField&& field);
    virtual DocSchema& AddStringField(StringField&& field);
    virtual DocSchema& AddFloatVectorField(FloatVectorField&& field);

    template <typename T>
    bool PreAddCheck(const T& field);

    DocSchema& AddLongFieldValue(const std::string& name, long value);
    DocSchema& AddFloatFieldValue(const std::string& name, float value);
    DocSchema& AddStringFieldValue(const std::string& name, const std::string& value);
    DocSchema& AddFloatVectorFieldValue(const std::string& name, const std::vector<float>& value);

    virtual std::string Dump() const;
    virtual bool Build();

    void Iterate(DocSchemaHandler* handler) const;

    bool HasBuilt() const { return fixed_; }

    uint64_t UID() const { return long_fields_[0].GetValue(); }

    uint8_t Size() const { return (uint8_t)fields_schema_.size(); }

    const std::string& GetFieldName(uint8_t field_id) const {
        assert(field_id < fields_id_name_.size());
        return fields_id_name_[field_id];
    }

    const PrimaryKeyT& GetPK() const { return long_fields_[PrimaryKeyIdx]; }

    bool GetFieldId(const std::string& field_name, uint8_t& field_id) const {
        const auto& it = fields_name_id_.find(field_name);
        if (it == fields_name_id_.end()) {
            return false;
        }
        field_id = it->second;
        return true;
    }

    bool GetFieldType(uint8_t field_id, uint8_t& field_type) {
        if (fields_id_type_.size() <= field_id) {
            return false;
        }
        field_type = fields_id_type_[field_id];
        return true;
    }

    bool GetFieldType(const std::string& field_name, uint8_t& field_type) {
        uint8_t field_id;
        auto check = GetFieldId(field_name, field_id);
        if (!check) return check;

        return GetFieldType(field_id, field_type);
    }

    rocksdb::Status Serialize(std::string& data) const;


    static rocksdb::Status Deserialize(const rocksdb::Slice& data, DocSchema& schema) {
        uint8_t fields_num = *(uint8_t*)(data.data());
        int offset = 1;
        uint8_t field_id;
        uint8_t field_type;
        uint8_t name_size;
        std::string field_name;
        while(fields_num-- > 0) {
            field_id = *(uint8_t*)(data.data() + offset++);
            field_type = *(uint8_t*)(data.data() + offset++);
            name_size = *(uint8_t*)(data.data() + offset++);

            field_name.assign(data.data()+offset, name_size);
            offset += name_size;
            /* std::cout << "field_id=" << (int)field_id << " field_type=" << (int)field_type; */
            /* std::cout << " field_name=" << field_name << std::endl; */

            // TODO: Store and fetch field parameters
            if (field_type == LongField::FieldTypeValue()) {
                LongField f(field_name);
                schema.AddLongField(std::move(f));
            } else if (field_type == StringField::FieldTypeValue()) {
                StringField f(field_name);
                schema.AddStringField(std::move(f));
            } else if (field_type == FloatField::FieldTypeValue()) {
                FloatField f(field_name);
                schema.AddFloatField(std::move(f));
            } else if (field_type == FloatVectorField::FieldTypeValue()) {
                FloatVectorField f(field_name);
                schema.AddFloatVectorField(std::move(f));
            }
        }

        schema.Build();

        return rocksdb::Status::OK();
    }

    virtual ~DocSchema() {}

protected:
    friend class Doc;
    friend class DumpHandler;

    std::map<std::string, std::pair<int, size_t>> fields_schema_;
    std::map<std::string, uint8_t> fields_name_id_;
    std::vector<std::string> fields_id_name_;
    std::vector<uint8_t> fields_id_type_;
    std::vector<LongField> long_fields_;
    std::vector<FloatField> float_fields_;
    std::vector<StringField> string_fields_;
    std::vector<FloatVectorField> float_vector_fields_;
    bool fixed_ = false;
};

class DocSchemaHandler {
public:
    virtual void PreHandle(const DocSchema& schema) = 0;
    virtual void Handle(const DocSchema& schema, const std::string& field_name, uint8_t field_id,
        int idx, size_t offset) = 0;
    virtual void PostHandle(const DocSchema& schema) = 0;
    virtual ~DocSchemaHandler() {}
};

class DumpHandler : public DocSchemaHandler {
public:
    void PreHandle(const DocSchema& schema) override;
    void Handle(const DocSchema& schema, const std::string& field_name, uint8_t field_id,
        int idx, size_t offset) override;
    void PostHandle(const DocSchema& schema) override;
    std::string ToString();

protected:
    std::stringstream ss_;
};

class DocSchemaSerializerHandler : public DocSchemaHandler {
public:
    void PreHandle(const DocSchema& schema) override;
    void Handle(const DocSchema& schema, const std::string& field_name, uint8_t field_id,
        int idx, size_t offset) override;
    void PostHandle(const DocSchema& schema) override;
    const std::string& ToString() const;
    std::string&& ToString();

protected:
    std::string serialized_;
};

class Doc : public DocSchema {
public:
    using BaseT = DocSchema;
    Doc(const PrimaryKeyT& pk, const std::shared_ptr<DocSchema> schema);
    Doc(PrimaryKeyT&& pk, const std::shared_ptr<DocSchema> schema);

    DocSchema& AddLongField(const LongField& field) override;
    DocSchema& AddFloatField(const FloatField& field) override;
    DocSchema& AddStringField(const StringField& field) override;
    DocSchema& AddFloatVectorField(const FloatVectorField& field) override;

    std::map<uint8_t, std::string> Serialize() const;

    bool Build() override;

private:
    std::shared_ptr<DocSchema> schema_;
};

class Helper {
public:
    template <typename ElementT>
    static VectorField<ElementT> BuildVectorField(size_t dimension) {
        VectorField<ElementT> vf;
        vf.SetMaxLength(dimension);
        vf.SetMinLength(dimension);
        return std::move(vf);
    }

    static DocSchema::PrimaryKeyT NewPK(typename DocSchema::PrimaryKeyT::ValueT value) {
        auto field = DocSchema::PrimaryKeyT(DocSchema::PrimaryKeyName);
        field.SetValue(value);
        field.Build();
        return std::move(field);
    }
};
