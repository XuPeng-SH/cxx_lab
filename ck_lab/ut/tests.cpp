#include <memory>
#include <gtest/gtest.h>
#include <iostream>
#include <atomic>
#include <random>

#include "IProcessor.h"
#include "Graph.h"
#include "Port.h"
#include "Pipe.h"
#include "Pipeline.h"
#include "Status.h"

using namespace std;
using namespace MyDB;

int
RandomInt(int start, int end) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(start, end);
    return dist(rng);
}

class MyUT : public ::testing::Test {
};

TEST_F(MyUT, Atomic) {
    {
        int old_aval = 1;
        std::atomic<int> aval = old_aval;
        int expected = 2;
        int desired = 3;

        while (!aval.compare_exchange_weak(expected, desired));
        ASSERT_EQ(aval, desired);
        ASSERT_EQ(expected, old_aval);
    }
}

TEST_F(MyUT, Port_State) {
    {
        Data data;
        data.val = 999;
        auto uint = State::GetUint(&data);
        auto ptr = State::GetPtr(uint);
        ASSERT_EQ(ptr->val, data.val);

    }
    {
        auto d1 = State::DataPtr();
        d1.data_->val = 11;
        std::atomic<Data*> adata(new Data());
        adata.load()->val = 22;
        auto uuint = d1.Swap(adata, State::HAS_DATA, State::HAS_DATA);
        ASSERT_EQ(uuint, 0x0);
        ASSERT_EQ(State::GetUint(d1.data_) & State::FLAGS_MASK, 0);
        ASSERT_EQ(State::GetUint(adata.load()) & State::FLAGS_MASK, State::HAS_DATA);

        uuint = d1.Swap(adata, State::IS_NEEDED, 0);
        ASSERT_EQ(State::GetUint(d1.data_) & State::FLAGS_MASK, 0);
        ASSERT_EQ(uuint, State::HAS_DATA);
        ASSERT_EQ(State::GetUint(adata.load()) & State::FLAGS_MASK, State::HAS_DATA | State::IS_NEEDED);

        uuint = d1.Swap(adata, State::IS_FINISHED, State::IS_NEEDED | State::HAS_DATA);
        ASSERT_EQ(State::GetUint(d1.data_) & State::FLAGS_MASK, 0);
        ASSERT_EQ(uuint, State::HAS_DATA | State::IS_NEEDED);
        ASSERT_EQ(State::GetUint(adata.load()) & State::FLAGS_MASK, State::IS_FINISHED);
    }
}

OutputPorts
CreateOutputPorts(int n = -1) {
    if (n < 0) {
        n = RandomInt(2, 5);
    }
    OutputPorts ports;
    for (int i = 0; i < n; ++i) {
        ports.emplace_back();
    }
    return std::move(ports);
}

InputPorts
CreateInputPorts(int n = -1) {
    if (n < 0) {
        n = RandomInt(2, 5);
    }
    InputPorts ports;
    for (int i = 0; i < n; ++i) {
        ports.emplace_back();
    }
    return std::move(ports);
}

TEST_F(MyUT, Port) {
    int num_outputs = RandomInt(2, 5);
    int num_inputs = RandomInt(2, 5);
    auto outputs = CreateOutputPorts(num_outputs);
    auto inputs = CreateInputPorts(num_inputs);
    ASSERT_EQ(num_outputs, outputs.size());
    ASSERT_EQ(num_inputs, inputs.size());
}

IProcessorPtr
CreateProcessor(size_t input_size, size_t output_size) {
    auto inputs = CreateInputPorts(input_size);
    auto outputs = CreateOutputPorts(output_size);
    return std::make_shared<IProcessor>(inputs, outputs);
}

GraphPtr
CreateGraph(Processors& processors) {
    if (processors.empty()) {
        return nullptr;
    }

    IProcessor* pre = processors[0].get();
    for (auto i = 1; i < processors.size(); ++i) {
        auto ret = pre->AsFromConnect(processors[i].get());
        if (!ret) return nullptr;
        pre = processors[i].get();
    }

    return std::make_shared<Graph>(processors);
}

TEST_F(MyUT, Graph1) {
    int chain_size = RandomInt(0, 4);

    vector<size_t> degrees = {0};
    for (auto i = 0; i < chain_size; ++i) {
        degrees.emplace_back(RandomInt(1, 4));
    }
    degrees.emplace_back(0);

    Processors processors;
    size_t input_size = degrees[0];
    size_t output_size = 0;

    for (auto i = 1; i < degrees.size(); ++i) {
        output_size = degrees[i];
        processors.push_back(CreateProcessor(input_size, output_size));
        input_size = output_size;
    }

    auto graph = CreateGraph(processors);
    ASSERT_TRUE(graph);
    std::cout << graph->ToString() << std::endl;

    /* for (auto& processor : processors) { */
    /*     delete processor; */
    /* } */
}

TEST_F(MyUT, Pipe1) {
    Pipe pipe;
    ASSERT_EQ(pipe.OutputPortSize(), 0);
    ASSERT_TRUE(pipe.Empty());

    auto source = CreateProcessor(0, 1);
    auto t1_output = RandomInt(1,4);
    auto t2_output = RandomInt(1,4);
    auto t1 = CreateProcessor(1, t1_output);
    auto t2 = CreateProcessor(t1_output, t2_output);
    auto sink = CreateProcessor(t2_output, 0);

    pipe.AddSource(source);
    size_t max_st = 1;
    ASSERT_EQ(pipe.NumOfProcessors(), 1);
    ASSERT_EQ(pipe.OutputPortSize(), source->OutputPortSize());
    ASSERT_EQ(pipe.MaxParallelStreams(), source->OutputPortSize());

    pipe.AddTransform(t1);
    max_st = std::max(max_st, t1->OutputPortSize());
    ASSERT_EQ(pipe.NumOfProcessors(), 2);
    ASSERT_EQ(pipe.OutputPortSize(), t1->OutputPortSize());
    ASSERT_EQ(pipe.MaxParallelStreams(), max_st);

    pipe.AddTransform(t2);
    max_st = std::max(max_st, t2->OutputPortSize());
    ASSERT_EQ(pipe.NumOfProcessors(), 3);
    ASSERT_EQ(pipe.OutputPortSize(), t2->OutputPortSize());
    ASSERT_EQ(pipe.MaxParallelStreams(), max_st);
    /* pipe.AddTransform(sink); */
}

TEST_F(MyUT, MutltiSourcePipe) {
    Pipe pipe;
    ASSERT_EQ(pipe.OutputPortSize(), 0);
    ASSERT_TRUE(pipe.Empty());

    auto source = CreateProcessor(0, 1);
    auto t1_output = RandomInt(1,4);
    auto t2_output = RandomInt(1,4);
    auto t1 = CreateProcessor(1, t1_output);
    auto t2 = CreateProcessor(t1_output, t2_output);
    auto sink = CreateProcessor(t2_output, 0);

    pipe.AddSource(source);
    size_t max_st = 1;
    ASSERT_EQ(pipe.NumOfProcessors(), 1);
    ASSERT_EQ(pipe.OutputPortSize(), source->OutputPortSize());
    ASSERT_EQ(pipe.MaxParallelStreams(), source->OutputPortSize());

    pipe.AddTransform(t1);
    max_st = std::max(max_st, t1->OutputPortSize());
    ASSERT_EQ(pipe.NumOfProcessors(), 2);
    ASSERT_EQ(pipe.OutputPortSize(), t1->OutputPortSize());
    ASSERT_EQ(pipe.MaxParallelStreams(), max_st);

    pipe.AddTransform(t2);
    max_st = std::max(max_st, t2->OutputPortSize());
    ASSERT_EQ(pipe.NumOfProcessors(), 3);
    ASSERT_EQ(pipe.OutputPortSize(), t2->OutputPortSize());
    ASSERT_EQ(pipe.MaxParallelStreams(), max_st);

    auto source2 = CreateProcessor(0, 1);
    pipe.AddSource(source2);
    ASSERT_EQ(pipe.NumOfProcessors(), 4);
    ASSERT_EQ(pipe.OutputPortSize(), t2->OutputPortSize() + source2->OutputPortSize());

    std::cout << pipe.ToString() << std::endl;

    auto t3_output = RandomInt(1,4);
    auto t3 = CreateProcessor(t2->OutputPortSize() + source2->OutputPortSize(), t3_output);
    pipe.AddTransform(t3);
    ASSERT_EQ(pipe.NumOfProcessors(), 5);
    ASSERT_EQ(pipe.OutputPortSize(), t3->OutputPortSize());
    /* pipe.AddTransform(sink); */
}

PipePtr
CreatePipe() {
    int chain_size = RandomInt(0, 4);

    vector<size_t> degrees = {0, 1};
    for (auto i = 0; i < chain_size; ++i) {
        degrees.emplace_back(RandomInt(1, 4));
    }

    Processors processors;
    size_t input_size = degrees[0];
    size_t output_size = 0;

    for (auto i = 1; i < degrees.size(); ++i) {
        output_size = degrees[i];
        processors.push_back(CreateProcessor(input_size, output_size));
        input_size = output_size;
    }

    auto pipe = std::make_shared<Pipe>();
    auto first = true;
    for (auto processor : processors) {
        if (first) {
            first = false;
            pipe->AddSource(processor);
        } else {
            pipe->AddTransform(processor);
        }
    }
    return pipe;
}

Pipes
CreatePipes(size_t n) {
    Pipes pipes;
    for (auto i = 0; i < n; ++i) {
        auto pipe = CreatePipe();
        pipes.push_back(pipe);
        /* std::cout << pipe->ToString() << std::endl; */
    }
    return std::move(pipes);
}

TEST_F(MyUT, MergePipes) {
    auto pipes = CreatePipes(RandomInt(2, 5));
    size_t pre_max = 0;
    size_t pre_num_processors = 0;
    size_t pre_output_size = 0;
    for (auto& p : pipes) {
        pre_max += p->MaxParallelStreams();
        pre_output_size += p->OutputPortSize();
        pre_num_processors += p->NumOfProcessors();
    }
    auto pipe = Pipe::MergePipes(pipes);
    cout << pipe->ToString() << endl;
    ASSERT_EQ(pre_max, pipe->MaxParallelStreams());
    ASSERT_EQ(pre_output_size, pipe->OutputPortSize());
    ASSERT_EQ(pre_num_processors, pipe->NumOfProcessors());

    auto output = RandomInt(1,4);
    auto t1 = CreateProcessor(pipe->OutputPortSize(), output);
    pipe->AddTransform(t1);
    ASSERT_EQ(pipe->NumOfProcessors(), pre_num_processors + 1);
    ASSERT_EQ(pipe->OutputPortSize(), t1->OutputPortSize());
    cout << pipe->ToString() << endl;
}

TEST_F(MyUT, Pipeline_basic) {
    auto pipe = CreatePipe();
    cout << pipe->ToString() << endl;
    PipelinePtr pipeline = std::make_shared<Pipeline>();
    ASSERT_FALSE(pipeline->IsInitialized());
    ASSERT_FALSE(pipeline->IsCompleted());
    auto empty_pipe = std::make_shared<Pipe>();
    auto status = pipeline->Initialize(empty_pipe);
    ASSERT_EQ(status.code(), PIPE_EMPTY);
    ASSERT_FALSE(pipeline->IsInitialized());

    status = pipeline->Initialize(pipe);
    ASSERT_TRUE(status.ok());
    ASSERT_TRUE(pipeline->IsInitialized());

    auto input_size = pipeline->NumStreams();
    auto output_size = RandomInt(1, 4);
    auto t1 = CreateProcessor(input_size, output_size);
    status = pipeline->AddTransform(t1);
    ASSERT_TRUE(status.ok());
    ASSERT_EQ(output_size, pipeline->NumStreams());
}
