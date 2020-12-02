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
    } else if (FLAGS_main == "i_u") {
        ret = ipc_utc_main();
    } else if (FLAGS_main == "p_i_u") {
        ret = pid_ipc_utc_main();
    } else if (FLAGS_main == "n_p_i_u") {
        ret = ns_pid_ipc_utc_main();
    } else {
        cout << "Error: Bad main=" << FLAGS_main << endl;
        cout << "Candidates: utc, i_u, p_i_u, n_p_i_u" << endl;
    }
    return ret;
}
