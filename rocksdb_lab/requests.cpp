#include "requests.h"
#include <iostream>

/* #define MARK std::cout << "[REQUEST ] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl */
/* #define MARK2 std::cout << "========[REQUEST ] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl */

#define MARK
#define MARK2

rocksdb::Status
BaseRequest::WaitToFinish() {
    MARK;
    std::unique_lock<std::mutex> lock(finish_mtx_);
    finish_cond_.wait(lock, [this] {return done_;});
    return rocksdb::Status::OK();
}

void
BaseRequest::Done() {
    MARK2;
    done_ = true;
    finish_cond_.notify_all();
}

rocksdb::Status
BaseRequest::Execute() {
    MARK2;
    status_ = OnExecute();
    Done();
    return status_;
}

BaseRequest::~BaseRequest() {
    MARK;
    WaitToFinish();
}

rocksdb::Status AddDocRequest::OnExecute() {
    MARK2;
    rocksdb::Status s;
    AddDocContextPtr context = std::dynamic_pointer_cast<AddDocContext>(context_);
    auto& table_name = context->GetTableName();
    auto& docs = context->GetDocs();
    auto db = context->GetDB();

    s = db->AddDocs(table_name, docs);
    if (!s.ok()) {
        std::cerr << s.ToString() << std::endl;
    }

    /* for (auto& doc : docs) { */
    /*     s = db->AddDoc(table_name, *doc); */
    /*     if (!s.ok()) { */
    /*         std::cerr << s.ToString() << std::endl; */
    /*         break; */
    /*     } */

    /* } */
    return s;
}

namespace lab {

void add_doc_request_lab() {
}

}
