#pragma once

#include <iostream>
#include <string>
#include <chrono>

using namespace std;

#define TIMING

#ifdef TIMING
#define INIT_TIMER auto start_time = std::chrono::high_resolution_clock::now();
#define START_TIMER  start_time = std::chrono::high_resolution_clock::now();
#define STOP_TIMER(name)  cout << "RUNTIME of " << name << ": " << \
    std::chrono::duration_cast<std::chrono::milliseconds>( \
            std::chrono::high_resolution_clock::now()-start_time \
    ).count() << " ms " << endl;
#else
#define INIT_TIMER
#define START_TIMER
#define STOP_TIMER(name)
#endif

namespace faiss {
    class Index;
    namespace gpu {
        class GpuResources;
    }
}

void search_index(faiss::Index* index, const string& context, int nq, int k,
        long nb, float *xb, int times, bool do_print = false);
/* void cpu_to_gpu_test(faiss::gpu::GpuResources* gpu_res, faiss::Index* index, const string& context, */
/*         long start, long end, long step); */
void cpu_to_gpu_test(faiss::gpu::GpuResources* gpu_res, faiss::Index* index, const string& context, int times);
