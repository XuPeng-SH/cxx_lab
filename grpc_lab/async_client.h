#pragma once
#include "demo.grpc.pb.h"
#include <string>
#include <memory>

using namespace demo::grpc;

using ::grpc::Channel;
using ::grpc::ChannelInterface;

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

     static std::shared_ptr<AsyncClient>
     Build(const std::string& host, const std::string& port);

 private:
     std::unique_ptr<MyService::Stub> stub_;
};
