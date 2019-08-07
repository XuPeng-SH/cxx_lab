#include "utils.h"
#include <faiss/index_io.h>
#include <faiss/IndexFlat.h>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/GpuIndex.h>
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/utils.h>
#include <faiss/AutoTune.h>
#include <faiss/gpu/GpuAutoTune.h>
#include <thread>
#include <sstream>

using namespace std;


void index_test(const string& index_type) {
    int times = 5;
    INIT_TIMER;
    START_TIMER;
    int d = 512;                            // dimension
    int nb = 100000;                       // database size
    START_TIMER;
    TestData data(d, nb);
    STOP_TIMER("Create Data: ");

    int gpu_num = 0;

    auto MSG_FUNC = [&](const string& msg) -> string {
        stringstream ss;
        ss << index_type << "_" << gpu_num << "_" << msg;
        return ss.str();
    };

    auto cpu_index = faiss::index_factory(d, index_type.c_str());

    {
        faiss::gpu::StandardGpuResources gpu_res;
        auto gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, gpu_num, cpu_index);
        delete cpu_index;

        START_TIMER;
        gpu_index->train(data.nb, data.xb);
        STOP_TIMER_WITH_FUNC("BuildIndex");

        START_TIMER;
        gpu_index->add(data.nb, data.xb);
        STOP_TIMER_WITH_FUNC("AddXB");
        cout << "gpu_index ntotal=" << gpu_index->ntotal << endl;

        search_index_test(gpu_index, MSG_FUNC("GPUSearchTest"), 200, 100, data.nb, data.xb, 5);

        START_TIMER;
        cpu_index = faiss::gpu::index_gpu_to_cpu(gpu_index);
        STOP_TIMER_WITH_FUNC("GpuToCpu");
        delete gpu_index;

        search_index_test(cpu_index, MSG_FUNC("CPUSearchTest"), 200, 100, data.nb, data.xb, 5);

    }
    {
        faiss::gpu::StandardGpuResources gpu_res;
        START_TIMER;
        auto gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, gpu_num, cpu_index);
        STOP_TIMER_WITH_FUNC("CpuToGpu");
        cout << "gpu_index ntotal=" << gpu_index->ntotal << endl;
        /* this_thread::sleep_for(chrono::seconds(20)); */
        delete gpu_index;
    }

    delete cpu_index;
}

void flat_test() {
    int times = 5;
    INIT_TIMER;
    START_TIMER;
    int d = 512;                            // dimension
    int nb = 1000000;                       // database size
    START_TIMER;
    TestData data(d, nb);
    STOP_TIMER("Create Data: ");

    faiss::IndexFlatL2 flat_l2(d);

    START_TIMER;
    flat_l2.add(data.nb, data.xb);
    STOP_TIMER("ADD CPU XB: ");

    search_index_test(&flat_l2, "CpuSearchTest", 200, 100, data.nb, data.xb, times);

    faiss::gpu::StandardGpuResources gpu_res;

    faiss::Index* cpu_index = &flat_l2;
    cpu_to_gpu_test(&gpu_res, cpu_index, "CpuToGpuTEST", 5);

    long start_nb = data.nb;
    long step_nb = -100000;
    long end_nb = 100000;
    gpu_add_vectors_test(&gpu_res, "GpuAddVectorsTest", 3, start_nb, end_nb, step_nb, data.xb, data.d);

    faiss::gpu::GpuIndexFlatL2 gpu_index_flat(&gpu_res, d);
    gpu_index_flat.add(data.nb, data.xb);
    search_index_test(&gpu_index_flat, "GpuSearchTest", 200, 100, data.nb, data.xb, times);
}

int main() {
    faiss::distance_compute_blas_threshold = 800;
    quantizer_cloner_test();
    flat_test();
    string index_type = "IVF16384,SQ8";
    index_test(index_type);
    index_type = "IVF16384,Flat";
    index_test(index_type);
}
