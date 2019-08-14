#include "utils.h"

using namespace std;
void index_test(TestOptions& options) {
    INIT_TIMER;
    if (!options.data) {
        START_TIMER;
        options.MakeData();
        STOP_TIMER("Create Data: ");
        if (!options.data) {
            cout << "Invalid TestOptions" << endl;
            return;
        }
    }

    int times = options.search_times;
    int gpu_num = options.gpu_num;
    auto data = options.data;

    auto MSG_FUNC = [&](const string& msg) -> string {
        stringstream ss;
        ss << options.index_type << "_" << gpu_num << "_" << msg;
        return ss.str();
    };

    auto cpu_index = options.index;

    {
        faiss::gpu::StandardGpuResources gpu_res;
        faiss::gpu::GpuClonerOptions clone_option;
        clone_option.useFloat16 = options.useFloat16;
        clone_option.useFloat16CoarseQuantizer = options.useFloat16;
        cpu_to_gpu_test(&gpu_res, cpu_index.get(), MSG_FUNC("CpuToGpuTEST"), 5);
        START_TIMER;
        auto gpu_index = faiss::gpu::index_cpu_to_gpu(&gpu_res, gpu_num, cpu_index.get(), &clone_option);
        STOP_TIMER_WITH_FUNC("CpuToGpu");

        auto ivf = dynamic_cast<faiss::gpu::GpuIndexIVF*>(gpu_index);
        if(ivf) {
            ivf->setNumProbes(options.nprobe);
        }
        auto cpu_ivf = dynamic_cast<faiss::IndexIVF*>(gpu_index);
        if (cpu_ivf){
            cout << "Warning: Expect GPU Index!" << endl;
            cpu_ivf->nprobe = options.nprobe;
        }

        search_index_test(gpu_index, MSG_FUNC("GPUSearchTest"), options.nq, options.k, data->nb, data->xb, times);

        START_TIMER;
        auto temp_cpu_index = faiss::gpu::index_gpu_to_cpu(gpu_index);
        STOP_TIMER_WITH_FUNC("GpuToCpu");
        delete gpu_index;

        cpu_ivf = dynamic_cast<faiss::IndexIVF*>(temp_cpu_index);
        if (cpu_ivf){
            cpu_ivf->nprobe = options.nprobe;
        }

        if (cpu_ivf) {
            search_index_test(temp_cpu_index, MSG_FUNC("CPUSearchTest"), options.nq, options.k, data->nb, data->xb, times);
        }
        delete temp_cpu_index;
    }
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

    /* search_index_test(&flat_l2, "CpuSearchTest", 100, 100, data.nb, data.xb, times); */

    faiss::gpu::StandardGpuResources gpu_res;

    faiss::Index* cpu_index = &flat_l2;
    cpu_to_gpu_test(&gpu_res, cpu_index, "CpuToGpuTEST", 5);

    long start_nb = data.nb;
    long step_nb = -100000;
    long end_nb = 100000;
    gpu_add_vectors_test(&gpu_res, "GpuAddVectorsTest", 3, start_nb, end_nb, step_nb, data.xb, data.d);

    faiss::gpu::GpuIndexFlatL2 gpu_index_flat(&gpu_res, d);
    gpu_index_flat.add(data.nb, data.xb);
    /* search_index_test(&gpu_index_flat, "GpuSearchTest", 100, 100, data.nb, data.xb, times); */
}

int main(int argc, char** argv) {
    gpu_ivf_sq_test();
    /* ivf_sq_test(); */
    /* flat_test(); */
    return 0;
    TestOptions options;
    if (argc >= 2) {
       options.nb = atoi(*(argv+1));
    }

    /* options.index.reset(faiss::read_index("/tmp/ivf_index")); */
    /* options.d = options.index->d; */
    /* options.nb = options.index->ntotal; */

    faiss::distance_compute_blas_threshold = 800;
    auto gpu_nums = faiss::gpu::getNumDevices();
    options.search_times = 5;
    vector<bool> float16_options  = {false};
    /* vector<string> indice_type = {"IVF16384,SQ8", "IVF16384,Flat", "Flat"}; */
    /* vector<string> indice_type = {"IVF16384,Flat", "Flat"}; */
    vector<string> indice_type = {"IVF16384,SQ8"};
    /* vector<string> indice_type = {"IVF16384,SQ8", "IVF16384,Flat"}; */
    for(auto& index_type : indice_type) {
        options.index_type = index_type;
        options.MakeIndex(true);
        for (auto i=0; i<gpu_nums; ++i) {
            options.gpu_num = i;
            for(auto use_float16 : float16_options) {
                options.useFloat16 = use_float16;
                cout << (use_float16?"Use Float16---------":"Use Float32--------------") << endl;
                index_test(options);
            }
        }
    }

    faiss::write_index(options.index.get(), "/tmp/ivf_index");
}