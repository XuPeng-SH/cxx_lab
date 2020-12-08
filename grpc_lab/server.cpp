#include "server.h"
#include "calldata.h"
#include <string>
#include <grpc/grpc.h>
#include <grpc++/grpc++.h>
#include <iostream>

using ::grpc::ServerBuilder;


ServerImpl::ServerImpl(const std::string& host, const std::string& port)
    : host_(host), port_(port) {
}

void
ServerImpl::Run() {
    std::string addr = host_ + ":"  + port_;
    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);

    cq_ = builder.AddCompletionQueue();

    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << addr << std::endl;
    HandleRpcs();
}

void
ServerImpl::HandleRpcs() {
    new CallData(&service_, cq_.get());
    void* tag;
    bool ok;
    while(cq_->Next(&tag, &ok)) {
        auto* data = static_cast<CallData*>(tag);
        if (ok) {
            data->proceed();
        } else {
            std::cout << "Cannot proceed CallData" << std::endl;
        }

    }
}

ServerImpl::~ServerImpl() {
    server_->Shutdown();
    cq_->Shutdown();
}
