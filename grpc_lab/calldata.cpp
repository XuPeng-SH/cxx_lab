#include "calldata.h"
#include <assert.h>
#include <string>

using ::grpc::Status;

CallData::CallData(MyService::AsyncService* service, ServerCompletionQueue* cq) :
    service_(service), cq_(cq), responder_(&ctx_) {
    proceed();
}

void
CallData::OnCreate() {
    assert(status_ == CREATE);
    status_ = PROCESS;
    service_->RequestGet(&ctx_, &request_, &responder_, cq_, cq_, this);
}

void
CallData::OnProcess() {
    assert(status_ == PROCESS);
    new CallData(service_, cq_);
    response_.set_index_type(request_.index_type());
    response_.set_index_name("index_" + std::to_string(request_.index_type()));
    status_ = FINISH;
    responder_.Finish(response_, Status::OK, this);
}

void
CallData::OnFinish() {
    std::cout << "Status=" << status_ << std::endl;
    assert(status_ == FINISH);
    delete this;
}

void
CallData::proceed() {
    if (status_ == CREATE) {
        OnCreate();
    } else if (status_ == PROCESS) {
        OnProcess();
    } else {
        OnFinish();
    }
}
