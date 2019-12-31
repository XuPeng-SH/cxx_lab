#include "server.h"
#include <string>
#include <iostream>
#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <memory>

#include "db.grpc.pb.h"

using namespace std;

static int ID = 0;

Server::Server(const std::string& port) : port_(port) {}

class ServiceImpl : public db::grpc::DBService::Service {
public:
    ::grpc::Status CreateTable(::grpc::ServerContext* context, const ::db::grpc::CreateTableParam* request, ::db::grpc::CreateTableResponse* response) override {
        cout << __func__ << ":" << request->name() << " " << request->dimension() << endl;
        response->mutable_table()->set_id(ID++);
        response->mutable_table()->set_name(request->name());
        return ::grpc::Status::OK;
    }

};

void Server::run() {
    ServiceImpl service;
    string addr = "0.0.0.0:" + port_;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    cout << "Server listening on " << addr << endl;

    server->Wait();
}
