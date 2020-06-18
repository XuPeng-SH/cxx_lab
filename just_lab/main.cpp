#include <iostream>
#include <thread>
#include <unistd.h>
#include <sstream>

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

int main() {
    float* repo;
    int dim = 8;
    long size = 8;
    MakeData(repo, dim, size);
    Print(repo, dim, size);
    delete [] repo;
    return 0;
}
