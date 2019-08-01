#include "utils.h"
#include <faiss/index_io.h>
#include <faiss/IndexFlat.h>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/GpuIndex.h>
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/utils.h>
#include <thread>

using namespace std;

void flat_test() {
    int times = 5;
    INIT_TIMER;
    START_TIMER;
    int d = 512;                            // dimension
    int nb = 1500000;                       // database size
    START_TIMER;
    TestData data(d, nb);
    STOP_TIMER("Create Data: ");

    faiss::IndexFlatL2 flat_l2(d);
    cout << std::boolalpha << flat_l2.is_trained << endl;

    START_TIMER;
    flat_l2.add(data.nb, data.xb);
    STOP_TIMER("ADD CPU XB: ");

    search_index_test(&flat_l2, "CpuSearchTest", 20, 10, data.nb, data.xb, times);

    faiss::gpu::StandardGpuResources gpu_res;

    faiss::Index* cpu_index = &flat_l2;
    cpu_to_gpu_test(&gpu_res, cpu_index, "CpuToGpuTEST", 5);

    long start_nb = data.nb;
    long step_nb = -100000;
    long end_nb = 100000;
    gpu_add_vectors_test(&gpu_res, "GpuAddVectorsTest", 3, start_nb, end_nb, step_nb, data.xb, data.d);

    faiss::gpu::GpuIndexFlatL2 gpu_index_flat(&gpu_res, d);
    gpu_index_flat.add(data.nb, data.xb);
    search_index_test(&gpu_index_flat, "GpuSearchTest", 20, 10, data.nb, data.xb, times);
}

int main() {
    faiss::distance_compute_blas_threshold = 800;
    flat_test();
}
