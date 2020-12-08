#include "async_client.h"
#include <grpc/grpc.h>
#include <grpc++/grpc++.h>

using ::grpc::ClientContext;
using ::grpc::CompletionQueue;
using ::grpc::Status;
using ::grpc::ClientAsyncResponseReader;
using ::grpc::InsecureChannelCredentials;

AsyncClient::AsyncClient(std::shared_ptr<ChannelInterface> channel) : stub_(MyService::NewStub(channel)) {}

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

    rpc->Finish(&reply, &status, (void*)1);

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
