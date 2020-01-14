#include "executor.h"
#include <iostream>


#define MARK std::cout << "[EXECUTOR] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl
#define MARK2 std::cout << "========[EXECUTOR] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl


rocksdb::Status RequestExector::HandleRequest(RequestPtr request) {
    MARK;
    if (!request) return rocksdb::Status::OK();
    Enqueue(request);

    if (request->IsAsync()) {
        return rocksdb::Status::OK();
    }

    return request->WaitToFinish();
}

RequestExector::~RequestExector() {
    Stop();
}

void RequestExector::Enqueue(RequestPtr request) {
    MARK;
    std::unique_lock<std::mutex> lk(mtx_);
    int target_exector = request->Routing(parallel_);
    if (target_exector == -1) {
        target_exector = current_;
        current_ = (current_ + 1) % parallel_;
    }
    auto it = executors_.find(target_exector);
    if (it == executors_.end()) {
        auto queue = std::make_shared<Queue>();
        auto t = std::make_shared<std::thread>(&RequestExector::WorkingThread, this, queue);
        auto executor = std::make_shared<Executor>(t, queue);
        executors_[target_exector] = executor;
    }

    executors_[target_exector]->execute_queue->Put(request);
}

void RequestExector::Stop() {
    if (stopped_) return;

    std::unique_lock<std::mutex> lk(mtx_);
    for (auto& it : executors_) {
        it.second->execute_queue->Put(nullptr);
    }

    for (auto& it : executors_) {
        if (!it.second->execute_thread) continue;
        it.second->execute_thread->join();
    }

    stopped_ = true;
    std::cout << "Executors stopped" << std::endl;
}

void RequestExector::WorkingThread(QueuePtr queue) {
    MARK2;
    if (!queue) return;

    while (true) {
        RequestPtr request = queue->Take();
        if (!request) {
            std::cout << "Stopping thread " << std::this_thread::get_id() << std::endl;
            break;
        }

        auto status = request->Execute();
        if (!status.ok()) {
            std::cerr << "Request execute error " << status.ToString() << std::endl;
        }
    };
}

namespace lab {

void request_exector_lab() {
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

    auto executor = std::make_shared<RequestExector>(10);
    auto call_request = [&](int num){

        auto context = std::make_shared<AddDocContext>(request_id, table_name);

        for (auto i=0; i<num; i++) {
            DocPtr mydoc = std::make_shared<Doc>(Helper::NewPK(i+10000), schema);
            mydoc->AddLongFieldValue("age", 10+i)
                 .AddStringFieldValue("uid", std::to_string(1000000+i))
                 .Build();

            context->AddDoc(mydoc);
        }

        auto request = AddDocRequest::Create(context);

        auto status = executor->HandleRequest(request);

        if (!status.ok()) {
            std::cerr << status.ToString() << std::endl;
        }
    };

    std::vector<std::thread> ts;
    for (auto i=0; i<10; ++i) {
        ts.push_back(std::thread(call_request, 100));
    }

    for (auto& t : ts) {
        t.join();
    }
}

}
