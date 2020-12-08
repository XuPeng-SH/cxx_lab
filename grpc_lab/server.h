#pragma once
#include "demo.grpc.pb.h"
#include <memory>
#include <string>

using namespace demo::grpc;
using ::grpc::ServerCompletionQueue;

using ::grpc::Server;

class ServerImpl final {
 public:
     ServerImpl(const std::string& host, const std::string& port);

     void
     Run();
     void
     ShutDown();

     ~ServerImpl();

 private:
     void
     HandleRpcs();

     std::string host_;
     std::string port_;

     std::unique_ptr<ServerCompletionQueue> cq_;
     MyService::AsyncService service_;
     std::unique_ptr<Server> server_;
};
