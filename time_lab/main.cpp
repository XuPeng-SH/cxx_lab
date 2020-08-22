#include <iostream>
#include <thread>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <atomic>

#include "monotime.h"

using namespace std;

int main(int argc, char** argv) {
    auto now = CoarseMonoClock::now();
    auto d = now.time_since_epoch();
    auto delta = chrono::nanoseconds(d).count();
    cout << delta << endl;
    /* cout << now.time_since_epoch() << endl; */
    std::atomic_int cnt = 8;
    int target = 10;
    int new_cnt = 108;
    while (!cnt.compare_exchange_weak(target, new_cnt)) {
        std::cout << "not exchanged " << " cnt=" << cnt << " target=" << target << std::endl;
    }
    std::cout << "exchanged " << " cnt=" << cnt << " target=" << target << std::endl;
    return 0;
}
