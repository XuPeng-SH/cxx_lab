#include "configuration.h"
#include "config.pb.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <cerrno>
#include <stdexcept>
#include <cstddef>
#include <google/protobuf/util/json_util.h>
#include <opentracing/noop.h>

#include <opentracing/mocktracer/json_recorder.h>
#include <opentracing/mocktracer/tracer.h>

using namespace opentracing;
using namespace opentracing::mocktracer;

static std::string read_file(const std::string& filename) {
    errno = 0;
    std::ifstream in{filename};
    if (!in.good()) {
        throw std::runtime_error{
            "Failed to open " + filename + " " + std::strerror(errno)
        };
    }

    in.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    return std::string{
        std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}
    };
}

static void mock_tracer(Configuration::Ptr config) {
    MockTracerOptions options;
    options.propagation_options.propagation_key = "demo-span-context";
    std::unique_ptr<std::ostringstream> output{new std::ostringstream{}};

    std::ostringstream& oss = *output;
    options.recorder = std::unique_ptr<Recorder>{
        new JsonRecorder{std::move(output)}};

    std::shared_ptr<Tracer> tracer{
        new MockTracer{std::move(options)}
    };

    config->tracer = tracer;
}


static void load_tracer(const demo::Config& config_protobuf, Configuration::Ptr config) {
    std::string error_message;
    auto handle_maybe = opentracing::DynamicallyLoadTracingLibrary(
            config_protobuf.tracer_library().c_str(), error_message);

    if (!handle_maybe) throw std::runtime_error{error_message};

    config->tracing_library_handle = std::move(*handle_maybe);

    std::string tracer_config_json;
    auto status = google::protobuf::util::MessageToJsonString(
            config_protobuf.tracer_configuration(), &tracer_config_json
    );

    if (!status.ok()) {
        throw std::runtime_error(status.ToString());
    }
    std::cout << "tracer_config_json=" << tracer_config_json << std::endl;

    auto tracer_maybe = config->tracing_library_handle.tracer_factory().MakeTracer(tracer_config_json.data(),
            error_message);

    if (!tracer_maybe) throw std::runtime_error{error_message};

    config->tracer = *tracer_maybe;
}

Configuration::~Configuration() {
    close();
}

void Configuration::close() {
    if(tracer) {
        tracer->Close();
    }
}

Configuration::Ptr Configuration::Load(const std::string& config_file) {
    Configuration::Ptr configuration = std::make_shared<Configuration>();
    demo::Config config;
    auto contents = read_file(config_file);
    std::cout << "contents: " << contents << std::endl;
    auto status = google::protobuf::util::JsonStringToMessage(contents, &config);
    if (!status.ok()) throw std::invalid_argument{"Invalid configuration " + status.ToString()};
    std::cout << config.host() << std::endl;
    std::cout << config.port() << std::endl;
    std::cout << config.tracer_library() << std::endl;

    if (config.tracer_library().empty())
        configuration->tracer = opentracing::MakeNoopTracer();
    else {
        load_tracer(config, configuration);
        /* mock_tracer(configuration); */
    }

    configuration->host = config.host();
    configuration->port = config.port();

    return configuration;
}
