#include "executor.h"
#include <iostream>
#include <chrono>
#include <random>
#include "rocksdb_util.h"
#include "rocksdb_impl.h"


/* #define MARK std::cout << "[EXECUTOR] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl */
/* #define MARK2 std::cout << "========[EXECUTOR] TID=" << std::this_thread::get_id() << ": " << __func__ << ":" << __LINE__ << std::endl */

#define MARK
#define MARK2

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

void
BuildVectors(int64_t n, int dim, std::vector<float>& vectors) {
    vectors.clear();
    vectors.resize(n * dim);
    float* data = vectors.data();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < dim; j++) data[dim * i + j] = drand48();
        data[dim * i] += i / 1.;
    }
}

void request_exector_lab() {
    int64_t nb = 1;
    int dim = 512;
    std::vector<float> xb;
    BuildVectors(nb, dim, xb);

    auto options = db::DefaultOpenOptions();

    rocksdb::DB *kvdb;
    rocksdb::DB::Open(*options, "/tmp/request_exector_lab", &kvdb);
    std::shared_ptr<rocksdb::DB> skvdb(kvdb);
    auto impl = std::make_shared<db::RocksDBImpl>(skvdb);
    auto thisdb = std::make_shared<db::MyDB>(impl);

    auto schema = std::make_shared<DocSchema>();
    schema->AddLongField(LongField("_id"));

    StringField uid_field("uid");
    LongField age_field("age");
    FloatVectorField vector_field("img_vec");

    uid_field.SetMaxLength(20);
    uid_field.SetMinLength(10);

    vector_field.SetMaxLength(dim);
    vector_field.SetMinLength(dim);

    schema->AddLongField(age_field)
           .AddStringField(uid_field)
           .AddFloatVectorField(vector_field)
           .Build();

    std::string table_name = __func__;
    std::string request_id = "rid1";

    thisdb->CreateTable(table_name, *schema);

    auto executor = std::make_shared<RequestExector>(2);
    auto call_request = [&](int num, int start=0){

        auto context = std::make_shared<AddDocContext>(thisdb, request_id, table_name);

        for (auto i=0; i<num; i++) {
            DocPtr mydoc = std::make_shared<Doc>(Helper::NewPK(i+10000), schema);
            mydoc->AddLongFieldValue("age", 10+i)
                 .AddStringFieldValue("uid", std::to_string(1000000+i+start))
                 .AddFloatVectorFieldValue("img_vec", xb)
                 .Build();

            context->AddDoc(mydoc);
        }

        auto request = AddDocRequest::Create(context);

        auto status = executor->HandleRequest(request);

        if (!status.ok()) {
            std::cerr << status.ToString() << std::endl;
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> ts;
    for (auto i=0; i<1; ++i) {
        ts.push_back(std::thread(call_request, 100, i*1000000));
    }

    for (auto& t : ts) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << __func__ << " takes " << std::chrono::duration<double, std::milli>(end-start).count() << std::endl;
    thisdb->Dump(true);

    std::vector<TablePtr> vec;
    thisdb->GetTables(vec);
}

}
