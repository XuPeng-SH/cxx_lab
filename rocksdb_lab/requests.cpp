#include "requests.h"
#include <iostream>

#define MARK std::cout << "[REQUEST ] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl
#define MARK2 std::cout << "========[REQUEST ] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl

rocksdb::Status
BaseRequest::WaitToFinish() {
    MARK;
    std::unique_lock<std::mutex> lock(finish_mtx_);
    finish_cond_.wait(lock, [this] {return done_;});
    return rocksdb::Status::OK();
}

void
BaseRequest::Done() {
    MARK2;
    done_ = true;
    finish_cond_.notify_all();
}

rocksdb::Status
BaseRequest::Execute() {
    MARK2;
    status_ = OnExecute();
    Done();
    return status_;
}

BaseRequest::~BaseRequest() {
    MARK;
    WaitToFinish();
}

rocksdb::Status AddDocRequest::OnExecute() {
    MARK2;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return rocksdb::Status::OK();
}

namespace lab {

void add_doc_request_lab() {
    auto schema = std::make_shared<DocSchema>();
    schema->AddLongField(LongField("_id"));

    StringField uid_field("uid");
    LongField age_field("age");

    uid_field.SetMaxLength(20);
    uid_field.SetMinLength(10);

    schema->AddLongField(age_field)
           .AddStringField(uid_field)
           .Build();

    std::string table_name = __func__;
    std::string request_id = "rid1";

    auto context = std::make_shared<AddDocContext>(request_id, table_name);

    int num = 10;
    for (auto i=0; i<num; i++) {
        DocPtr mydoc = std::make_shared<Doc>(Helper::NewPK(i+10000), schema);
        mydoc->AddLongFieldValue("age", 10+i)
             .AddStringFieldValue("uid", std::to_string(1000000+i))
             .Build();

        context->AddDoc(mydoc);
    }

    auto request = AddDocRequest::Create(context);

    auto status = request->Execute();
    if (!status.ok()) {
        std::cerr << status.ToString() << std::endl;
    }
}

}
