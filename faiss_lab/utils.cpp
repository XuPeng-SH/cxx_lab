#include "utils.h"
#include <faiss/Index.h>
#include <sstream>
#include <vector>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/GpuAutoTune.h>
#include <faiss/gpu/GpuIndexFlat.h>
/* #include <faiss/gpu/GpuIndexIVFQuantizer.h> */
#include <faiss/Clustering.h>
#include <faiss/OnDiskInvertedLists.h>
#include <gpu/impl/IVFSQ.cuh>
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/IndexScalarQuantizer.h>
#include <faiss/gpu/GpuIndexIVFSQ.h>
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
    index->display();
    INIT_TIMER;
    faiss::indexIVF_stats.reset();
    long *I = new long[k * nq];
    float *D = new float[k * nq];
    for (auto i=0; i<times; i++) {
        START_TIMER;
        index->search(nq, xb, k, D, I);
        STOP_TIMER(ss.str());
        if (do_print) {
            printf("I=\n");
            for(int i = 0; i < nq; i++) {
                for(int j = 0; j < k; j++) {
                    if (I[i * k + j] != -1) {
                        printf("%5ld ", I[i * k + j]);
                    }
                }
                printf("\n");
            }
            /* printf("D=\n"); */
            /* for(int i = 0; i < nq; i++) { */
            /*     for(int j = 0; j < k; j++) printf("%5f ", D[i * k + j]); */
            /*     printf("\n"); */
            /* } */
        }
    }
    ss.str("");
    ss << "NQ=" << faiss::indexIVF_stats.nq;
    ss << " NL=" << faiss::indexIVF_stats.nlist;
    ss << " ND=" << faiss::indexIVF_stats.ndis;
    ss << " NH=" << faiss::indexIVF_stats.nheap_updates;
    ss << " Q=" << faiss::indexIVF_stats.quantization_time;
    ss << " S=" << faiss::indexIVF_stats.search_time;
    cout << ss.str() << endl;
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

void
gpu_ivf_sq_test() {
    INIT_TIMER;
    TestFactory options;
    options.d = 4;
    options.nb = 20;
    options.index_type = "IVF8,SQ8";
    options.MakeIndex();
    auto cpu_index = options.index.get();

    auto MSG_FUNC = [&](const string& msg) -> string {
        stringstream ss;
        ss << options.index_type << "_" << options.gpu_num << "_" << msg;
        return ss.str();
    };

    auto ivf_sq = dynamic_cast<faiss::IndexIVFScalarQuantizer*>(cpu_index);
    if (!ivf_sq) {
        cout << "Expect faiss::IndexIVFScalarQuantizer!" << endl;
        return;
    }
    if (options.nb < 100) {
        auto lists = ivf_sq->invlists;
        for (auto i=0; i<ivf_sq->nlist; ++i) {
            auto numVecs = ivf_sq->get_list_size(i);
            auto ids = lists->get_ids(i);
            cout << "CpuIDS[" << i << "]=";
            for (auto j=0; j<numVecs; ++j) {
                cout << " " << *(ids+j);
            }
            cout << endl;
        }
        for (auto i=0; i<ivf_sq->nlist; ++i) {
            auto numVecs = ivf_sq->get_list_size(i);
            auto codes = lists->get_codes(i);
            cout << "CpuData[" << i << "]=";
            for (auto j=0; j<numVecs*ivf_sq->d; ++j) {
                cout << " " << (unsigned)(*(codes+j));
            }
            cout << endl;
        }
    }

    faiss::gpu::StandardGpuResources gpu_res;
    faiss::gpu::CpuToGpuClonerOptions clone_option;
    clone_option.readonly = true;
    auto gpu_index = faiss::gpu::cpu_to_gpu(&gpu_res, 0, ivf_sq, &clone_option);
    auto sq = dynamic_cast<faiss::gpu::GpuIndexIVFSQ*>(gpu_index);
    if (options.nb < 100) {
        sq->dump();
    }
    delete gpu_index;

    START_TIMER;
    gpu_index = faiss::gpu::cpu_to_gpu(&gpu_res, 0, ivf_sq, &clone_option);
    STOP_TIMER_WITH_FUNC("ReadonlyCpuToGpu")

    cout << gpu_index->ntotal << endl;

    delete gpu_index;

}

void
ivf_sq_test() {
#if 0
    TestFactory options;
    options.d = 128;
    options.nb = 20;
    options.index_type = "IVF10,SQ8";
    options.MakeIndex();
    auto cpu_index = options.index.get();

    auto ivf_sq = dynamic_cast<faiss::IndexIVFScalarQuantizer*>(cpu_index);
    if (!ivf_sq) {
        cout << "Expect faiss::IndexIVFScalarQuantizer!" << endl;
        return;
    }

    faiss::gpu::StandardGpuResources gpu_res;
    /* auto gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, 0, cpu_index); */
    /* delete cpu_index; */

    auto quan_gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, 0, ivf_sq->quantizer);
    auto flat_quan_gpu_index = dynamic_cast<faiss::gpu::GpuIndexFlat*>(quan_gpu_index);

    faiss::gpu::IndicesOptions indicesOptions = faiss::gpu::INDICES_64_BIT;
    auto gpu_core = std::make_shared<faiss::gpu::IVFSQ>(&gpu_res, flat_quan_gpu_index->getGpuData(),
            ivf_sq->sq.code_size, true, indicesOptions, faiss::gpu::Device);


    auto lists = ivf_sq->invlists;

    for (auto i=0; i<ivf_sq->nlist; ++i) {
        auto numVecs = ivf_sq->get_list_size(i);
        cout << "numVecs[" << i << "]=" << numVecs << endl;
        gpu_core->addCodeVectorsFromCpu(
                i, lists->get_codes(i), lists->get_ids(i), numVecs
                );
    }


    for (auto i=0; i<gpu_core->getNumLists(); ++i) {
        cout << "GpunumVecs[" << i << "]=" << gpu_core->getListLength(i) << endl;
    }
    for (auto i=0; i<ivf_sq->nlist; ++i) {
        auto numVecs = ivf_sq->get_list_size(i);
        auto ids = lists->get_ids(i);
        cout << "CpuIDS[" << i << "]=";
        for (auto j=0; j<numVecs; ++j) {
            cout << " " << *(ids+j);
        }
        cout << endl;
    }

    for (auto i=0; i<gpu_core->getNumLists(); ++i) {
        auto indices = gpu_core->getListIndices(i);
        cout << "GpuIDS[" << i << "]=";
        for (auto& indice : indices) {
            cout << " " << indice;
        }
        cout << endl;

    }

    delete quan_gpu_index;
#endif
}

void inverted_list_test(TestFactory& factory) {
    auto ivf_index = dynamic_pointer_cast<faiss::IndexIVF>(factory.index);
    if (!ivf_index) {
        return;
    }
    faiss::ReadOnlyArrayInvertedLists* lists = dynamic_cast<faiss::ReadOnlyArrayInvertedLists*>(ivf_index->invlists->to_readonly());
    faiss::ArrayInvertedLists* ail = dynamic_cast<faiss::ArrayInvertedLists*>(ivf_index->invlists);
    std::vector<long> raw_ids;
    for (auto& list_ids : ail->ids) {
        raw_ids.insert(raw_ids.end(), list_ids.begin(), list_ids.end());
    }
    std::vector<uint8_t> raw_codes;
    for (auto& list_codes : ail->codes) {
        raw_codes.insert(raw_codes.end(), list_codes.begin(), list_codes.end());
    }

    auto print_ids = [](const std::string& lable, std::vector<long> data) {
        cout << lable;
        for (auto id : data) {
            cout << " " << id;
        }
        cout << endl;
    };

    auto print_codes = [](const std::string& lable, std::vector<uint8_t>& data) {
        cout << lable;
        for (auto id : data) {
            cout << " " << (unsigned)id;
        }
        cout << endl;
    };
    cout << lists->readonly_length.size() << endl;
    cout << lists->readonly_offset.size() << endl;
    cout << lists->readonly_ids.size() << endl;
    cout << lists->readonly_codes.size() << endl;

    cout << "CodeSize: " << lists->code_size << endl;
    assert(raw_ids == lists->readonly_ids);
    assert(raw_codes == lists->readonly_codes);
    assert(ail->nlist == lists->nlist);
    assert(ail->code_size == lists->code_size);
#if 0

    for (auto i=0; i<ail->nlist; ++i) {
        auto size = lists->list_size(i);
        cout << "list " << i << " size " << size << endl;
        auto codes = lists->get_codes(i);
        cout << "codes " << i << " :";
        for (auto j=0; j<size*ail->code_size; ++j) {
            cout << " " << (unsigned)*(codes+j);
        }
        cout << endl;
        auto ids = lists->get_ids(i);
        cout << "ids " << i << " :";
        for (auto j=0; j<size; ++j) {
            cout << " " << *(ids+j);
        }
        cout << endl;
    }
#endif


    /* string fname = "/tmp/readonly_lists"; */
    string fname = "/tmp/normal_lists";

    /* print_ids("RAWIDS=", raw_ids); */
    /* print_ids("ROIDS=", lists->readonly_ids); */
    /* print_codes("RAWCODES=", raw_codes); */
    /* print_codes("ROCODES=", lists->readonly_codes); */

    /* ivf_index->replace_invlists(lists, true); */
    /* ivf_index->to_readonly(); */
    cout << ivf_index->is_readonly() << endl;
    faiss::write_index(factory.index.get(), fname.c_str());

    auto loaded_index = faiss::read_index(fname.c_str());

    faiss::gpu::StandardGpuResources gpu_res;
    auto gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, 0, loaded_index);

    delete gpu_index;

    delete loaded_index;
}

void quantizer_cloner_test() {
#if 0
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
#endif
}
