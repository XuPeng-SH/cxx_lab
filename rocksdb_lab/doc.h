#pragma once

#include "fields.h"
#include <vector>
#include <map>
#include <type_traits>
#include <memory>

class Doc;
class DocSchemaHandler;
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

    uint8_t Size() const { return (uint8_t)fields_schema_.size(); }

    const PrimaryKeyT& GetPK() const { return long_fields_[PrimaryKeyIdx]; }

    virtual ~DocSchema() {}

protected:
    friend class Doc;
    friend class DumpHandler;

    std::map<std::string, std::pair<int, size_t>> fields_schema_;
    std::map<std::string, uint8_t> fields_name_id_;
    std::vector<std::string> fields_id_name_;
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

class Doc : public DocSchema {
public:
    using BaseT = DocSchema;
    Doc(const PrimaryKeyT& pk, const std::shared_ptr<DocSchema> schema);
    Doc(PrimaryKeyT&& pk, const std::shared_ptr<DocSchema> schema);

    DocSchema& AddLongField(const LongField& field) override;
    DocSchema& AddFloatField(const FloatField& field) override;
    DocSchema& AddStringField(const StringField& field) override;
    DocSchema& AddFloatVectorField(const FloatVectorField& field) override;

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
