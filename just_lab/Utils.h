#pragma once

#include <chrono>

#define TIMING

#ifdef TIMING
#define INIT_TIMER auto start_time = std::chrono::high_resolution_clock::now();
                   /* auto MSG_FUNC = [&](const string& msg) -> string {return msg;}; */
#define START_TIMER  start_time = std::chrono::high_resolution_clock::now();
#define STOP_TIMER_WITH_FUNC(name)  cout << "RUNTIME of " << MSG_FUNC(name) << ": " << \
    std::chrono::duration_cast<std::chrono::microseconds>( \
            std::chrono::high_resolution_clock::now()-start_time \
    ).count() << " us " << endl;
#define STOP_TIMER(name)  cout << "RUNTIME of " << name << ": " << \
    std::chrono::duration_cast<std::chrono::microseconds>( \
            std::chrono::high_resolution_clock::now()-start_time \
    ).count() << " us " << endl;
#else
#define INIT_TIMER
#define START_TIMER
#define STOP_TIMER(name)
#endif

int RandomInt(int start, int end);
