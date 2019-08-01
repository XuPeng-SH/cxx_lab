#include "utils.h"
#include <faiss/index_io.h>
#include <faiss/IndexFlat.h>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/GpuIndex.h>
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/utils.h>
#include <thread>

using namespace std;

void demo() {
    INIT_TIMER;
    START_TIMER;
    int d = 512;                            // dimension
    int nb = 1500000;                       // database size
    int nq = 10000;                        // nb of queries
    float *xb = new float[d * nb];
    float *xq = new float[d * nq];
    for(int i = 0; i < nb; i++) {
        for(int j = 0; j < d; j++) xb[d * i + j] = drand48();
        xb[d * i] += i / 1000.;
    }
    for(int i = 0; i < nq; i++) {
        for(int j = 0; j < d; j++) xq[d * i + j] = drand48();
        xq[d * i] += i / 1000.;
    }
    STOP_TIMER("Create Data: ");

    faiss::IndexFlatL2 flat_l2(d);
    cout << std::boolalpha << flat_l2.is_trained << endl;

    START_TIMER;
    flat_l2.add(nb, xb);
    STOP_TIMER("ADD CPU XB: ");

    cout << "ntotal=" << flat_l2.ntotal << endl;
    string context = "CPU";
    int times = 2;
    search_index(&flat_l2, context, 20, 10, nb, xb, times);

    faiss::Index* cpu_index = &flat_l2;
    faiss::gpu::StandardGpuResources gpu_res;

    /* START_TIMER; */
    /* auto gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, 0, cpu_index); */
    /* STOP_TIMER("Cpu->Gpu: "); */
    /* cout << "gpu_index is_trained=" << std::boolalpha << gpu_index->is_trained << endl; */
    /* cout << "gpu_index ntotal=" << std::boolalpha << gpu_index->ntotal << endl; */
    /* delete gpu_index; */

    cpu_to_gpu_test(&gpu_res, cpu_index, "CPU->GPU TEST", 5);

    long start_nb = 200000;
    long step_nb = 200000;
    long end_nb = nb;

    faiss::gpu::GpuIndexFlatL2 gpu_index_flat(&gpu_res, d);

    START_TIMER;
    gpu_index_flat.add(nb, xb);
    STOP_TIMER("gpu index add: ");
    cout << "gpu_index is_trained=" << std::boolalpha << gpu_index_flat.is_trained << endl;
    cout << "gpu_index ntotal=" << std::boolalpha << gpu_index_flat.ntotal << endl;

    context = "GPU";

    search_index(&gpu_index_flat, context, 20, 10, nb, xb, times);

    delete [] xb;
    delete [] xq;
}

int main() {
    faiss::distance_compute_blas_threshold = 800;
    cout << "Hello" << endl;
    demo();
}
