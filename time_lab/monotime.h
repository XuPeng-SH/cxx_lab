#include <chrono>

class CoarseMonoClock {
 public:
    using duration = std::chrono::nanoseconds;
    /* using Duration = duration; */
    using time_point = std::chrono::time_point<CoarseMonoClock>;
    /* using TimePoint = time_point; */

    /* static constexpr bool is_steady = true; */

    static time_point now();
    /* static TimePoint Now() { */
    /*     return now(); */
    /* } */
};
