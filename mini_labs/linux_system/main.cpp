#include <iostream>
#include <unistd.h>
#include "utils.h"

using namespace std;

int main(int argc, char** argv) {
    cout << "hello system lab" << endl;

    redirect_std_streams();

    cout << "1. Get system processors number "
        << sysconf(_SC_NPROCESSORS_CONF) << endl;
    cout << "2. Get system processors number currently online "
        << sysconf(_SC_NPROCESSORS_ONLN) << endl;
    cout << "3. Get system pagesize "
        << sysconf(_SC_PAGESIZE) << endl;
    cout << "4. Get system number of pages "
        << sysconf(_SC_PHYS_PAGES) << endl;
    cout << "5. Get system avaliable number of pages "
        << sysconf(_SC_AVPHYS_PAGES) << endl;
    cout << "5. Get system max files opened "
        << sysconf(_SC_OPEN_MAX) << endl;


    return 0;
}
