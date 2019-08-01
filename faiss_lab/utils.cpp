#include "utils.h"
#include <faiss/Index.h>
#include <sstream>
#include <vector>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/GpuAutoTune.h>
#include <assert.h>

void search_index(faiss::Index* index, const string& context, int nq, int k, long nb, float *xb, int times, bool do_print) {
    stringstream ss;
    ss << "Search " << context << " nq=" << nq << " topk=" << k << " nb=" << nb;
    INIT_TIMER;
    long *I = new long[k * nq];
    float *D = new float[k * nq];
    for (auto i=0; i<times; i++) {
        START_TIMER;
        index->search(nq, xb, k, D, I);
        STOP_TIMER(ss.str());
        if (do_print) {
            printf("I=\n");
            for(int i = 0; i < nq; i++) {
                for(int j = 0; j < k; j++) printf("%5ld ", I[i * k + j]);
                printf("\n");
            }
            printf("D=\n");
            for(int i = 0; i < nq; i++) {
                for(int j = 0; j < k; j++) printf("%5f ", D[i * k + j]);
                printf("\n");
            }
        }
    }
    delete [] I;
    delete [] D;
}
/* void cpu_to_gpu_test(faiss::gpu::GpuResources* gpu_res, const string& context, long start, long end, long step, float* xb) { */
/*     assert(end >= start); */
/*     assert(step >= 0); */
/*     INIT_TIMER; */
/*     for (long nb = start; nb <= end; nb += step) { */
/*         stringstream ss; */
/*         ss << context << " nb=" << nb; */
/*         START_TIMER; */
/*         auto gpu_index = faiss::gpu::index_cpu_to_gpu(gpu_res, 0, index); */
/*         ss << " ntotal=" << gpu_index->ntotal; */
/*         STOP_TIMER(ss.str()); */
/*         delete gpu_index; */
/*     } */
/* } */

void cpu_to_gpu_test(faiss::gpu::GpuResources* gpu_res, faiss::Index* index, const string& context, int times) {
    INIT_TIMER;
    for (auto i = 0; i <= times; i += 1) {
        stringstream ss;
        ss << context;
        START_TIMER;
        auto gpu_index = faiss::gpu::index_cpu_to_gpu(gpu_res, 0, index);
        ss << " ntotal=" << gpu_index->ntotal;
        STOP_TIMER(ss.str());
        delete gpu_index;
    }
}
