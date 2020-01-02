#pragma once

#include "fields.h"
#include <vector>
#include <map>
#include <type_traits>
#include <memory>

class Doc;
class DocSchema {
public:
    using PrimaryKeyT = LongField;
    static const char* PrimaryKeyName;

    DocSchema(const PrimaryKeyT& pk = PrimaryKeyT(PrimaryKeyName));
    DocSchema(PrimaryKeyT&& pk);

    DocSchema(DocSchema&& other);

    virtual DocSchema& AddLongField(const LongField& field);
    virtual DocSchema& AddFloatField(const FloatField& field);
    virtual DocSchema& AddStringField(const StringField& field);
    virtual DocSchema& AddFloatVectorField(const FloatVectorField& field);

    virtual std::string Dump() const;
    virtual bool Build();

    bool HasBuilt() const { return fixed_; }

    const PrimaryKeyT& GetPK() const { return long_fields_[PrimaryKeyIdx]; }

    virtual ~DocSchema() {}

protected:
    friend class Doc;
    static const int LongFieldIdx;
    static const int StringFieldIdx;
    static const int FloatFieldIdx;
    static const int PrimaryKeyIdx;
    static const int FloatVectorFieldIdx;

    std::map<std::string, std::pair<int, size_t>> fields_schema_;
    std::vector<LongField> long_fields_;
    std::vector<FloatField> float_fields_;
    std::vector<StringField> string_fields_;
    std::vector<FloatVectorField> float_vector_fields_;
    bool fixed_ = false;
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

    static DocSchema::PrimaryKeyT&& NewPK(typename DocSchema::PrimaryKeyT::ValueT value) {
        auto field = DocSchema::PrimaryKeyT(DocSchema::PrimaryKeyName);
        field.SetValue(value);
        field.Build();
        return std::move(field);
    }
};
