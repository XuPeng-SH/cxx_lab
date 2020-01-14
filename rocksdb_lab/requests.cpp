#include "requests.h"

rocksdb::Status
BaseRequest::WaitToFinish() {
    std::unique_lock<std::mutex> lock(finish_mtx_);
    finish_cond_.wait(lock, [this] {return done_;});
}

void
BaseRequest::Done() {
    std::unique_lock<std::mutex> lock(finish_mtx_);
    finish_cond_.notify_all();
}

rocksdb::Status
BaseRequest::Execute() {
    status_ = OnExecute();
    Done();
    return status_;
}

BaseRequest::~BaseRequest() {
    WaitToFinish();
}
