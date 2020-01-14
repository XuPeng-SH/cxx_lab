#pragma once

#include <string>
#include <rocksdb/status.h>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include "doc.h"

/* using namespace document; */
using DocPtr = std::shared_ptr<Doc>;


class BaseContext {
public:
    explicit BaseContext(const std::string& request_id, bool async = false) : request_id_(request_id), async_(async) {}

    const std::string& RequestID() const { return request_id_; }

    bool IsAsync() const { return async_; }

protected:
    std::string request_id_;
    bool async_;
};

using ContextPtr = std::shared_ptr<BaseContext>;

class BaseRequest {
public:
    using Ptr = std::shared_ptr<BaseRequest>;

    rocksdb::Status Execute();
    void Done();
    rocksdb::Status WaitToFinish();

    virtual int Routing(size_t pool_size) { return -1; }

    const rocksdb::Status& Status() const {
        return status_;
    }

    bool IsAsync() const { return context_->IsAsync(); }

    virtual ~BaseRequest();

protected:
    BaseRequest(const ContextPtr& context) : context_(context) {}
    virtual rocksdb::Status OnExecute() = 0;

    mutable std::mutex finish_mtx_;
    std::condition_variable finish_cond_;

    bool done_ = false;
    rocksdb::Status status_;
    ContextPtr context_;
};

using Request = BaseRequest;
using RequestPtr = BaseRequest::Ptr;

class AddDocContext : public BaseContext {
public:
    AddDocContext(const std::string& request_id, const std::string& table_name, const std::vector<DocPtr>& docs = {})
        : BaseContext(request_id),
          table_name_(table_name),
          docs_(docs) {
    }

    const std::string& GetTableName() const {
        return table_name_;
    }

    void AddDoc(DocPtr doc) {
        docs_.push_back(doc);
    }

    const std::vector<DocPtr> GetDocs() const {
        return docs_;
    }

protected:
    std::string table_name_;
    std::vector<DocPtr> docs_;
};


class AddDocRequest : public BaseRequest {
public:
    static RequestPtr
    Create(const ContextPtr& context) {
        return RequestPtr(new AddDocRequest(context));
    }

protected:
    AddDocRequest(const ContextPtr& context) : BaseRequest(context) {}

    rocksdb::Status
    OnExecute() override;

};

namespace lab {
    void add_doc_request_lab();
}
