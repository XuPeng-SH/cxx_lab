#pragma once
#include "demo.grpc.pb.h"
#include <string>
#include <memory>
#include <thread>

using namespace demo::grpc;

using ::grpc::Channel;
using ::grpc::ChannelInterface;
using ::grpc::CompletionQueue;
using ::grpc::ClientContext;
using ::grpc::Status;

struct Printable {
    virtual void
    Dump() const = 0;
};

template <typename RespT>
struct RpcResponse : Printable {
    using Ptr = std::shared_ptr<RpcResponse<RespT>>;
    long request_id;
    RespT reply;
    ClientContext ctx;
    Status status;
    std::string
    ToString() const {
        return "Response of request " + std::to_string(request_id);
    }
    void
    Dump() const override {
        std::cout << ToString() << std::endl;
    }
};

struct IndexResult {
    int index;
    std::string name;
};
using IndexResultPtr = std::shared_ptr<IndexResult>;

class AsyncClient {
 public:
     explicit AsyncClient(std::shared_ptr<ChannelInterface> channel);

     IndexResultPtr
     GetIndex(int index);
     RpcResponse<Index>*
     AsyncGetIndex(long request_id, int index);

     static std::shared_ptr<AsyncClient>
     Build(const std::string& host, const std::string& port);

     void
     Run();
     void
     Stop();

     ~AsyncClient();

 private:
     void
     DoRun();

     std::unique_ptr<MyService::Stub> stub_;
     CompletionQueue cq_;
     /* ClientContext ctx_; */
     std::thread loop_;
};
