#include <iostream>
#include <gflags/gflags.h>
#include "ns.h"

using namespace std;

DEFINE_string(main, "utc", "Specify pgm to run");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    int ret = 0;
    if (FLAGS_main == "utc") {
        ret = utc_main();
    } else if (FLAGS_main == "ipc_utc") {
        ret = ipc_utc_main();
    }
    return ret;
}
