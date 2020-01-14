#pragma once

#include <string>
#include <rocksdb/status.h>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include "advanced_doc.h"

using namespace document;

class BaseRequest {
public:
    rocksdb::Status Execute();
    void Done();
    rocksdb::Status WaitToFinish();

    const rocksdb::Status& Status() const {
        return status_;
    }

    virtual ~BaseRequest();

protected:
    virtual rocksdb::Status OnExecute() = 0;


    mutable std::mutex finish_mtx_;
    std::condition_variable finish_cond_;

    bool done_ = false;
    rocksdb::Status status_;
};

class BaseContext {
public:
    explicit BaseContext(const std::string& request_id) : request_id_(request_id) {}

    const std::string& RequestID() const { return request_id_; }

protected:
    std::string request_id_;
};

class AddDocContext : public BaseContext {
public:
    AddDocContext(const std::string& request_id, const std::string& table_name, const std::vector<DocPtr>& docs)
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

using RequestPtr = std::shared_ptr<BaseRequest>;

/* class AddDocRequest : public BaseRequest { */
/*     static RequestPtr */
/*     Create(const ) */
/* public: */
/*     AddDocRequest() */

/* }; */
