#include "server.h"
#include <string>
#include <iostream>
#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <memory>
#include <rocksdb/db.h>

#include "db.grpc.pb.h"
#include "database.h"
#include "rocksdb_impl.h"

using namespace std;

static int ID = 0;

Server::Server(const std::string& port, const std::string& db_path) : port_(port), db_path_(db_path) {}

class ServiceImpl : public db::grpc::DBService::Service {
public:
    ServiceImpl(const string& db_path, std::shared_ptr<DocSchema> schema) : schema_(schema) {
        rocksdb::Options options;
        options.create_if_missing = true;
        rocksdb::DB *kvdb;
        rocksdb::DB::Open(options, db_path, &kvdb);
        std::shared_ptr<rocksdb::DB> skvdb(kvdb);
        auto impl = std::make_shared<db::RocksDBImpl>(skvdb);
        db_ = std::make_shared<db::MyDB>(impl);
    }

    ::grpc::Status CreateTable(::grpc::ServerContext* context, const ::db::grpc::CreateTableParam* request, ::db::grpc::CreateTableResponse* response) override {
        db_->CreateTable(request->name(), *schema_);
        response->mutable_table()->set_id(ID++);
        response->mutable_table()->set_name(request->name());
        return ::grpc::Status::OK;
    }

private:
    std::shared_ptr<DocSchema> schema_;
    std::shared_ptr<db::MyDB> db_;

};

void Server::run() {
    ServiceImpl service(db_path_, std::make_shared<DocSchema>());
    string addr = "0.0.0.0:" + port_;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    cout << "Server listening on " << addr << endl;

    server->Wait();
}
