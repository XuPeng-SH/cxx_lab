
#include "server.h"
#include <string>
#include <iostream>

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <opentracing/mocktracer/tracer.h>
#include <unistd.h>
#include <vector>
#include <memory>

#include "demo.grpc.pb.h"
#include "text_map_carrier.h"
#include "interceptor.h"

using namespace std;
using namespace grpc;
using namespace demo::grpc;


void ServerCallB(std::shared_ptr<opentracing::Tracer> tracer, const opentracing::SpanContext& ctx) {
    auto span = tracer->StartSpan(__func__, {ChildOf(&ctx)});
    usleep(300);
    span->Finish();
}

void ServerCallA(std::shared_ptr<opentracing::Tracer> tracer, const opentracing::SpanContext& ctx) {
    auto span = tracer->StartSpan(__func__, {ChildOf(&ctx)});
    usleep(1000);
    ServerCallB(tracer, ctx);
    usleep(200);
    span->SetTag("error", true);
    span->SetTag("message", "fake error");
    span->Finish();
}

class ServiceImpl : public MyService::Service {
public:
    explicit ServiceImpl(std::shared_ptr<opentracing::Tracer> tracer) : tracer_(tracer) {}
    Status Get(::grpc::ServerContext* context,
            const ::demo::grpc::QueryParam* request, ::demo::grpc::Index* response) override {
        /* auto& value = span_maybe.value(); */
        /* assert(span_maybe); */

        /* auto a = span_maybe->get(); */

        /* auto span = tracer_->StartSpan("ServerGet", {ChildOf(span_maybe->get())}); */
        /* span->SetTag("method", __func__); */
        /* usleep(500); */

        /* ServerCallA(tracer_, span->context()); */

        cout << "request.index_type=" << request->index_type() << endl;
        response->set_index_name("IndexName");
        response->set_index_type(request->index_type());
        /* span->Log({ */
        /*         {"name", "IndexName"}, */
        /*         {"type", request->index_type()} */
        /*         }); */
        /* span->Finish(); */
        return Status::OK;
    }

private:
    std::shared_ptr<opentracing::Tracer> tracer_;
};

void run_server(Configuration::Ptr configuration) {

    ServiceImpl service(configuration->tracer);
    std::string addr = configuration->host + ":" + configuration->port;
    ServerBuilder builder;
    builder.AddListeningPort(addr, InsecureServerCredentials());
    builder.RegisterService(&service);

    using InterceptorI = experimental::ServerInterceptorFactoryInterface;
    using InterceptorIPtr = std::unique_ptr<InterceptorI>;
    vector<InterceptorIPtr> creators;
    /* vector<std::unique_ptr<experimental::ServerInterceptorFactoryInterface>> creators; */
    creators.push_back(std::unique_ptr<experimental::ServerInterceptorFactoryInterface>(
                new SpanInterceptorFactory(configuration->tracer)));

    builder.experimental().SetInterceptorCreators(std::move(creators));

    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "Server listening on " << addr << std::endl;

    server->Wait();
}
