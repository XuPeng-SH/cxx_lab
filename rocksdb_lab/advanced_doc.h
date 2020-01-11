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

    virtual ThisT& AddField(std::shared_ptr<FieldT> field) {
        std::unique_lock<std::shared_timed_mutex> lock(mtx_);
        std::string name = "XXX";
        auto& it = fname_fid_.find(name);
        /* auto& it = fname_fid_.find(field->Name()); */
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
        return *this;
    }

    virtual ~BaseDocT() {}

protected:
    mutable std::shared_timed_mutex mtx_;
    std::map<std::string, std::shared_ptr<FieldT>> fname_field_;
    std::vector<std::shared_ptr<FieldT>> fid_field_;
    std::map<std::string, FID_T> fname_fid_;
};

template <typename FieldT>
class DocT : public BaseDocT<FieldT> {
public:
    using ThisT = DocT<FieldT>;
};

using Doc = DocT<Field>;

}
