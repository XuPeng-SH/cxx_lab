#pragma once

#include <string>
#include <memory>
#include <grpcpp/channel.h>
#include <grpc++/grpc++.h>
#include "db.grpc.pb.h"


class Client {
public:
    Client(std::shared_ptr<grpc::Channel> channel);

    std::shared_ptr<db::grpc::CreateTableResponse> CreateTable(const std::string& table_name);

    static std::shared_ptr<Client> Build(const std::string& ip, const std::string& port);

private:
    std::unique_ptr<::db::grpc::DBService::Stub> stub_;
};
