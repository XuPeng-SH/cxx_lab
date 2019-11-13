#include <opentracing/mocktracer/json_recorder.h>
#include <opentracing/mocktracer/tracer.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <gflags/gflags.h>
#include "text_map_carrier.h"
#include "server.h"
#include "client.h"
#include "configuration.h"

using namespace std;
using namespace opentracing;
using namespace opentracing::mocktracer;

DEFINE_string(mode, "server", "server or client");
DEFINE_string(host, "127.0.0.1", "host");
DEFINE_string(port, "5666", "port");
DEFINE_string(config, "", "config file location");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    cout << "FLAGS_mode: " << FLAGS_mode << endl;
    cout << "FLAGS_host: " << FLAGS_host << endl;
    cout << "FLAGS_port: " << FLAGS_port << endl;
    cout << "FLAGS_config: " << FLAGS_config << endl;

    auto configuration = Configuration::Load(FLAGS_config);

    if (FLAGS_mode == "server") {
        run_server(configuration);
    } else if (FLAGS_mode == "client") {
        auto client = make_client(configuration);
        auto resp = client->Get(1);
        cout << resp.name << endl;
        cout << resp.type << endl;
    }
    else {
        cerr << "FLAGS_mode: " << FLAGS_mode << " is invalid!" << endl;
        assert(false);
    }

    return 0;
    MockTracerOptions options;
    std::unique_ptr<std::ostringstream> output{new std::ostringstream{}};

    std::ostringstream& oss = *output;
    options.recorder = std::unique_ptr<mocktracer::Recorder>{
        new JsonRecorder{std::move(output)}};

    std::shared_ptr<opentracing::Tracer> tracer{
        new MockTracer{std::move(options)}
    };

    auto parent_span = tracer->StartSpan("parent");
    assert(parent_span);

    {
    /* } */

    /* { */
        std::unordered_map<std::string, std::string> text_map;
        TextMapCarrier carrier(text_map);

        auto err = tracer->Inject(parent_span->context(), carrier);
        assert(err);
        /* std::cout << "Inject error message: \"" << err.error().message() << "\"\n"; */
        for (auto i : text_map) {
            std::cout << i.first << ": ";
            std::cout << i.second << std::endl;
        }

        auto span_context_maybe = tracer->Extract(carrier);
        assert(span_context_maybe);

        auto span = tracer->StartSpan("childA", {ChildOf(span_context_maybe->get())});
        assert(span);
        span->Log({{"status", 0}, {"name", "childA"}});
        span->Finish();

        err = tracer->Inject(span->context(), carrier);
        /* assert(!err); */
        for (auto i : text_map) {
            std::cout << i.first << ": ";
            std::cout << i.second << std::endl;
        }

        /* std::unordered_map<std::string, std::string> text_map = { */
        /*     {"x-ot-span-context", "123"} */
        /* }; */
        /* TextMapCarrier carrier(text_map); */

        /* auto err = tracer->Extract(carrier); */
        /* std::cout << "Example error message: \"" << err.error().message() << "\"\n"; */
    }

    parent_span->Finish();
    tracer->Close();

    std::cout << oss.str() << std::endl;


    return 0;
}
