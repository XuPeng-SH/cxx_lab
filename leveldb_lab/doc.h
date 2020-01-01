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

    DocSchema(const PrimaryKeyT& pk = PrimaryKeyT());

    virtual DocSchema& AddLongField(const std::string& name, const LongField& field);
    virtual DocSchema& AddFloatField(const std::string& name, const FloatField& field);
    virtual DocSchema& AddStringField(const std::string& name, const StringField& field);

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

    std::map<std::string, std::pair<int, size_t>> fields_schema_;
    std::vector<LongField> long_fields_;
    std::vector<FloatField> float_fields_;
    std::vector<StringField> string_fields_;
    bool fixed_ = false;
};

class Doc : public DocSchema {
public:
    using BaseT = DocSchema;
    Doc(const PrimaryKeyT& pk, const std::shared_ptr<DocSchema> schema);

    DocSchema& AddLongField(const std::string& name, const LongField& field) override;
    DocSchema& AddFloatField(const std::string& name, const FloatField& field) override;
    DocSchema& AddStringField(const std::string& name, const StringField& field) override;

    bool Build() override;

private:
    std::shared_ptr<DocSchema> schema_;
};
