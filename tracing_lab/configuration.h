#pragma once
#include <string>
#include <opentracing/dynamic_load.h>
#include <opentracing/tracer.h>
#include <memory>

struct Configuration {
    using Ptr =  std::shared_ptr<Configuration>;
    static Configuration::Ptr Load(const std::string& config_file);
    opentracing::DynamicTracingLibraryHandle tracing_library_handle;
    std::shared_ptr<opentracing::Tracer> tracer;
    std::string host = "127.0.0.1";
    std::string port = "5666";

    void close();

    ~Configuration();
};
