#include "async_client.h"
#include <grpc/grpc.h>
#include <grpc++/grpc++.h>
#include <chrono>

using ::grpc::ClientContext;
using ::grpc::Status;
using ::grpc::ClientAsyncResponseReader;
using ::grpc::InsecureChannelCredentials;

AsyncClient::AsyncClient(std::shared_ptr<ChannelInterface> channel) : stub_(MyService::NewStub(channel)) {}

AsyncClient::~AsyncClient() {
    loop_.join();
}

RpcResponse<Index>*
AsyncClient::AsyncGetIndex(long request_id, int index) {
    QueryParam request;
    request.set_index_type(index);

    auto* response = new RpcResponse<Index>();
    response->request_id = request_id;
    std::unique_ptr<ClientAsyncResponseReader<Index>> rpc(stub_->PrepareAsyncGet(&response->ctx, request, &cq_));
    rpc->StartCall();
    rpc->Finish(&(response->reply), &(response->status), (void*)(response));

    return response;
}

void
AsyncClient::Stop() {
    cq_.Shutdown();
}

void
AsyncClient::Run() {
    loop_ = std::thread(std::bind(&AsyncClient::DoRun, this));
}

void
AsyncClient::DoRun() {
    void* got_tag;
    bool ok = false;

    while (cq_.Next(&got_tag, &ok)) {
        if (!ok) {
            std::cout << "not ok" << std::endl;
        }
        auto pt = (Printable*)(got_tag);
        pt->Dump();
        delete pt;
    }
    std::cout << "Stopped" << std::endl;
}

IndexResultPtr
AsyncClient::GetIndex(int index) {
    QueryParam request;
    request.set_index_type(index);

    Index reply;
    ClientContext ctx;

    CompletionQueue cq;
    Status status;

    std::unique_ptr<ClientAsyncResponseReader<Index>> rpc(stub_->PrepareAsyncGet(&ctx, request, &cq));

    rpc->StartCall();
    auto start = std::chrono::high_resolution_clock::now();

    rpc->Finish(&reply, &status, (void*)1);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Call takes " << std::chrono::duration<double, std::milli>(end-start).count() << " ms" << std::endl;

    void* got_tag;
    bool ok = false;

    auto ret = cq.Next(&got_tag, &ok);
    assert(got_tag == (void*) 1);
    assert(ok);

    if (status.ok()) {
        auto ret = std::make_shared<IndexResult>();
        ret->index = reply.index_type();
        ret->name = reply.index_name();
        return ret;
    } else {
        return nullptr;
    }
}

std::shared_ptr<AsyncClient>
AsyncClient::Build(const std::string& host, const std::string& port) {
    std::string addr = host + ":" + port;
    auto cre = InsecureChannelCredentials();
    auto channel = CreateChannel(addr, cre);
    auto client = std::make_shared<AsyncClient>(channel);
    return client;
}
