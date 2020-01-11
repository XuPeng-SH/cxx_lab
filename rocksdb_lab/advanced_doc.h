#pragma once
#include "advanced_fields.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <rocksdb/status.h>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace advanced {

using FID_T = uint8_t;

template <typename FieldT>
class BaseDocT {
public:
    using ThisT = BaseDocT<FieldT>;

    virtual bool AddField(std::shared_ptr<FieldT> field) {
        std::unique_lock<std::shared_timed_mutex> lock(mtx_);
        auto it = fname_fid_.find(field->Name());
        if (it == fname_fid_.end()) {
            FID_T fid = fid_field_.size();
            fname_fid_[field->Name()] = fid;
            fid_field_.push_back(field);
            fname_field_[field->Name()] = field;
        } else {
            auto& fid = it->second;
            fid_field_[fid] = field;
            fname_field_[field->Name()] = field;
        }
        return true;
    }

    const std::shared_ptr<FieldT> GetField(const std::string& name) const {
        std::shared_lock<std::shared_timed_mutex> lock(mtx_);
        auto it = fname_field_.find(name);
        if(it != fname_field_.end()) {
            return it->second;
        }
        return nullptr;
    }

    const std::shared_ptr<FieldT> GetField(FID_T fid) const {
        std::shared_lock<std::shared_timed_mutex> lock(mtx_);
        if (fid >= fid_field_.size()) return nullptr;
        return fid_field_[fid];
    }

    const std::shared_ptr<FieldT> GetPK() const {
        return GetField(0);
    }

    bool GetFieldId(const std::string& field_name, FID_T& fid) const {
        std::shared_lock<std::shared_timed_mutex> lock(mtx_);
        auto it = fname_fid_.find(field_name);
        if (it != fname_fid_.end()) {
            fid = it->second;
            return true;
        }

        return false;
    }

    virtual ~BaseDocT() {}

protected:
    mutable std::shared_timed_mutex mtx_;
    std::map<std::string, std::shared_ptr<FieldT>> fname_field_;
    std::vector<std::shared_ptr<FieldT>> fid_field_;
    std::map<std::string, FID_T> fname_fid_;
};

template <typename FieldT>
class DocSchemaT : public BaseDocT<FieldT> {
public:
    using ThisT = DocSchemaT<FieldT>;
    using BaseT = BaseDocT<FieldT>;
};

using DocSchema = DocSchemaT<Field>;

template <typename FieldT>
class DocT : public BaseDocT<FieldT> {
public:
    using ThisT = DocT<FieldT>;
    using BaseT = BaseDocT<FieldT>;
    using SchemaT = DocSchemaT<FieldT>;

    DocT(std::shared_ptr<SchemaT> schema) : schema_(schema) {}

    bool AddField(const std::shared_ptr<FieldT> field) override {
        auto schema_field = schema_->GetField(field->Name());
        if (!schema_field) {
            std::cerr << "Cannot add field with name \"" << field->Name() << "\"" << std::endl;
            return false;
        }
        return BaseT::AddField(field);
    }

protected:
    std::shared_ptr<DocSchemaT<FieldT>> schema_;

};

using Doc = DocT<Field>;

}
