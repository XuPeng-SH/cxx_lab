#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <time.h>
#include <thread>

using namespace std;

struct MutexLatch {
    union {
        struct {
            volatile unsigned char xcl[1];
            volatile unsigned char filler;
            volatile unsigned short waiters[1];
        } bits[1];
        unsigned int value[1];
    };
};

void mutexlock(MutexLatch* latch) {
    unsigned int idx, waited = 0;
    MutexLatch prev[1];
    prev->value[0] = 0;
    while(1) {
        for (idx=0; idx<100; idx++) {
            *prev->value = __sync_fetch_and_or(latch->value, 1);
            if (!*prev->bits->xcl) {
                if (waited)
                    __sync_fetch_and_sub(latch->bits->waiters, 1);
                return;
            }

            if (!waited) {
                __sync_fetch_and_add(latch->bits->waiters, 1);
                *prev->bits->waiters += 1;
                waited++;
            }

            cout << "tid=" << std::this_thread::get_id() << " is waiting" << endl;
            syscall(SYS_futex, latch->value, FUTEX_WAIT, *prev->value, NULL, NULL, 0);
        }
    }
}

void releasemutex(MutexLatch* latch) {
    MutexLatch prev[1];
    prev->value[0] = 0;
    *prev->value = __sync_fetch_and_and(latch->value, 0xffff0000);
    if (*prev->bits->waiters) {
        cout << "tid=" << std::this_thread::get_id() << " is releasing" << endl;
        syscall(SYS_futex, latch->value, FUTEX_WAKE, 1, NULL, NULL, 0);
    }
}

void demo(const string& name, int sec, MutexLatch* latch) {
    mutexlock(latch);
    cout << name << " is runing" << endl;
    sleep(sec);
    releasemutex(latch);
}


int main(int argc, char** argv) {
    MutexLatch latch[1];
    latch->value[0] = 0;
    auto t1 = std::thread(demo, "t1", 2, latch);
    sleep(1);
    auto t2 = std::thread(demo, "t2", 2, latch);
    t1.join();
    t2.join();
    return 0;
}
