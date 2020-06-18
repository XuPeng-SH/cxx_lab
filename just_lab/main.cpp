#include <iostream>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <gflags/gflags.h>
#include <assert.h>

#include "Utils.h"

using namespace std;

void MakeData(float*& repo, int dim, long size) {
    repo = new float[dim * size];
    for (auto i=0; i<size; ++i) {
        for (auto j=0; j<dim; ++j) {
            repo[dim*i+j] = drand48();
        }
        repo[dim*i] += i/1000.;
    }
}

void Print(float*& repo, int dim, long size) {
    stringstream ss;
    for (auto i=0; i<size; ++i) {
        for (auto j=0; j<dim; ++j) {
            /* repo[dim*i+j] = drand48(); */
            ss << repo[dim*i+j] << ", ";
        }
        ss << "\n";
    }
    std::cout << ss.str() << std::endl;
}

DEFINE_int32(dim, 8, "dim of vecs");
DEFINE_int64(size, 8, "row size");
DEFINE_string(input, "", "input file");
DEFINE_string(output, "", "output file");
DEFINE_bool(mock_data, false, "mock data");
DEFINE_int32(slice, 100, "slice size");

int main(int argc, char** argv) {
    INIT_TIMER;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    float* repo;
    int dim = FLAGS_dim;
    long size = FLAGS_size;
    if (FLAGS_mock_data) {
        assert(FLAGS_output != "");
        MakeData(repo, dim, size);
        if (dim*size <= 200) {
            Print(repo, dim, size);
        }
        ofstream f(FLAGS_output, ios::binary);
        f.write((char*)(&dim), sizeof(dim));
        f.write((char*)(&size), sizeof(size));
        f.write((char*)repo, sizeof(float) * dim * size);
        f.close();
        cout << "Dump outfile: " << FLAGS_output << endl;
    } else {
        assert(FLAGS_input != "");
        cout << "Input file: " << FLAGS_input;
        ifstream f(FLAGS_input, ios::binary);
        f.read((char*)(&dim), sizeof(dim));
        f.read((char*)(&size), sizeof(size));
        cout << " dim=" << dim << " size=" << size << endl;

        repo = new float[dim*size];
        START_TIMER;
        /* f.read((char*)(repo), sizeof(float)*dim*size); */
        auto left = size;
        auto pos = 0;
        auto slice = FLAGS_slice;
        auto cnt = 0;
        while (left > 0) {
            auto to_load = (left >= slice) ? slice : left;
            f.read((char*)(repo + pos*sizeof(float)), sizeof(float)*dim*to_load);
            pos += to_load;
            left -= to_load;
            ++cnt;
        }
        STOP_TIMER("Load");
        cout << "Execute " << cnt << " times" << endl;
        if (dim*size <= 200) {
            Print(repo, dim, size);
        }
        f.close();
    }
    delete [] repo;
    return 0;
}
