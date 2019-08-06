#include "utils.h"
#include <faiss/Index.h>
#include <sstream>
#include <vector>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/GpuAutoTune.h>
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/gpu/GpuIndexIVFQuantizer.h>
#include <faiss/Clustering.h>
#include <faiss/OnDiskInvertedLists.h>
#include <assert.h>


TestData::TestData(int dimension, long size)
    : d(dimension), nb(size) {
    xb = new float[d * nb];
    for(int i = 0; i < nb; i++) {
        for(int j = 0; j < d; j++) xb[d * i + j] = drand48();
        xb[d * i] += i / 1000.;
    }
}

TestData::~TestData() {
    delete [] xb;
}


void search_index_test(faiss::Index* index, const string& context, int nq, int k, long nb, float *xb, int times, bool do_print) {
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

void gpu_add_vectors_test(faiss::gpu::GpuResources* gpu_res, const string& context, int times, long start, long end, long step, float* xb, int d) {
    assert((end >= start && step >= 0) || (end <= start && step <= 0));
    INIT_TIMER;
    auto expect_le = [](long lh, long rh) -> bool {
        return lh <= rh;
    };
    auto expect_ge = [](long lh, long rh) -> bool {
        return lh >= rh;
    };

    auto func = (start < end) ? expect_le : expect_ge;

    for (auto i=0; i<times; ++i) {
        /* for (long nb = start; nb<=end; nb += step) { */
        for (long nb = start; func(nb, end); nb += step) {
            faiss::gpu::GpuIndexFlatL2 gpu_index_flat(gpu_res, d);
            stringstream ss;
            ss << context << " nb=" << nb;
            START_TIMER;
            gpu_index_flat.add(nb, xb);
            ss << " ntotal=" << gpu_index_flat.ntotal;
            STOP_TIMER(ss.str());
        }
    }
}

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

void quantizer_cloner_test() {
    faiss::gpu::StandardGpuResources gpu_res;
    int d = 512;
    auto cpu_index = faiss::index_factory(d, "IVF16384,Flat");

    int nb = 200000;
    TestData data(d, nb);
    auto gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, 0, cpu_index);
    delete cpu_index;

    gpu_index->train(data.nb, data.xb);
    gpu_index->add(data.nb, data.xb);

    cout << gpu_index->ntotal << endl;
    cout << dynamic_cast<faiss::gpu::GpuIndexIVF*>(gpu_index)->getQuantizer()->ntotal << endl;

    cpu_index = faiss::gpu::index_gpu_to_cpu(gpu_index);
    delete gpu_index;

    cout << cpu_index->ntotal << endl;
    cout << dynamic_cast<faiss::IndexIVF*>(cpu_index)->quantizer->ntotal << endl;

    faiss::gpu::GpuClonerOptions clone_option;
    clone_option.storeInvertedList = false;

    gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, 0, cpu_index, &clone_option);
    auto gpu_ivf_quan = dynamic_cast<faiss::gpu::GpuIndexFlat*>(gpu_index);
    cout << gpu_ivf_quan->getNumVecs() << endl;
    cout << gpu_ivf_quan->ntotal << endl;
    cout << gpu_ivf_quan->d << endl;

    search_index_test(gpu_index, "GpuQuantizerTest", 5, 3, data.nb, data.xb, 2, true);
    delete gpu_index;

    delete cpu_index;
}
