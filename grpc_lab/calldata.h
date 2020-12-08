#pragma once
#include "demo.grpc.pb.h"

using namespace demo::grpc;
using ::grpc::ServerCompletionQueue;
using ::grpc::ServerContext;
using ::grpc::ServerAsyncResponseWriter;

class CallData {
 public:
     CallData(MyService::AsyncService* service, ServerCompletionQueue* cq);

     void proceed();

 private:
     void
     OnCreate();
     void
     OnProcess();
     void
     OnFinish();

     MyService::AsyncService* service_;
     ServerCompletionQueue* cq_;
     ServerContext ctx_;

     QueryParam request_;
     Index response_;
     ServerAsyncResponseWriter<Index> responder_;

     enum CallStatus { CREATE, PROCESS, FINISH };
     CallStatus status_ = CREATE;
};
