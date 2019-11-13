#include "client.h"
#include "text_map_carrier.h"
#include <unistd.h>

using namespace std;
using namespace grpc;


void ClientCallC(std::shared_ptr<opentracing::Tracer> tracer, const opentracing::SpanContext& ctx) {
    auto span = tracer->StartSpan(__func__, {ChildOf(&ctx)});
    usleep(600);
    span->Finish();
}

std::shared_ptr<Client> make_client(Configuration::Ptr configuration) {
    std::string addr = configuration->host + ":" + configuration->port;
    auto client = std::make_shared<Client>(CreateChannel(addr, InsecureChannelCredentials()),
            configuration->tracer);
    return client;
}

Client::Client(std::shared_ptr<Channel> channel, std::shared_ptr<opentracing::Tracer> tracer)
    : stub_(demo::grpc::MyService::NewStub(channel)), tracer_(tracer) {

}

ThisIndex Client::Get(int type) {
    ::demo::grpc::QueryParam query;
    ::demo::grpc::Index index;
    query.set_index_type(type);
    grpc::ClientContext context;

    ThisIndex ret;

    auto span = tracer_->StartSpan("ClientGet");
    span->SetTag("request-id", 11111);

    ClientCallC(tracer_, span->context());


    std::unordered_map<std::string, std::string> text_map;
    TextMapCarrier carrier(text_map);

    auto err = tracer_->Inject(span->context(), carrier);
    for (auto i : text_map) {
        std::cout << i.first << ": ";
        std::cout << i.second << std::endl;
    }

    context.AddMetadata("demo-span-context", text_map["demo-span-context"]);

    auto status = stub_->Get(&context, query, &index);
    if (!status.ok()) {
        cerr << status.error_code() << endl;
        return ret;
    }
    ClientCallC(tracer_, span->context());

    ret.name = index.index_name();
    ret.type = index.index_type();
    span->Finish();
    return ret;
}
