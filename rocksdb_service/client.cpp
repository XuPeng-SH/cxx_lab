#include "client.h"
#include <iostream>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>


using namespace std;

Client::Client(shared_ptr<grpc::Channel> channel)
     : stub_(::db::grpc::DBService::NewStub(channel)) {
}

std::shared_ptr<Client> Client::Build(const std::string& ip, const std::string& port) {
    std::string addr = ip + ":" + port;
    auto client = std::make_shared<Client>(grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()));
    return client;
}

std::shared_ptr<db::grpc::CreateTableResponse> Client::CreateTable(const std::string& table_name) {
    cout << __func__ << ": " << table_name <<endl;
    grpc::ClientContext context;
    db::grpc::CreateTableParam param;
    param.set_name(table_name);
    param.set_dimension(512);

    auto response = std::make_shared<db::grpc::CreateTableResponse>();
    auto status = stub_->CreateTable(&context, param, response.get());
    if (!status.ok()) {
        cerr << status.error_code() << endl;
    }
    return response;
}
