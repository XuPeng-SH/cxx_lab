#pragma once
#include <string>
#include <iostream>
#include <memory>

#include <grpc++/grpc++.h>
#include "demo.grpc.pb.h"
#include "configuration.h"

struct ThisIndex {
    std::string name;
    int type;
};

class Client {
public:
    explicit Client(std::shared_ptr<grpc::Channel> channel, std::shared_ptr<opentracing::Tracer> tracer);

    ThisIndex Get(int type);

private:
    std::unique_ptr<::demo::grpc::MyService::Stub> stub_;
    std::shared_ptr<opentracing::Tracer> tracer_;

};

std::shared_ptr<Client> make_client(Configuration::Ptr configuration);
