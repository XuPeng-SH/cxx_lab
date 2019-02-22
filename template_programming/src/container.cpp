#include "container.hpp"
#include <algorithm>
#include <iostream>
#include <functional>

void find_if_demo() {
    std::vector<int> numbers = {
        4,11,2,5,6,10
    };

    int ceil = 8;

    auto found_it = std::find_if(numbers.begin(), numbers.end(),
            std::bind2nd(std::less<int>(), ceil));

    /* 2RD */
    /* auto less_func = [&](int number) -> bool { */
    /*     return number < ceil; */
    /* }; */
    /* auto found_it = std::find_if(numbers.begin(), numbers.end(), less_func); */

    /* 3RD */
    /* auto found_it = std::find_if(numbers.begin(), numbers.end(), [&](int number) -> bool { */
    /*     return number < ceil; */
    /* }); */

    if (found_it == numbers.end()) {
        std::cout << "Ceil=" << ceil << "Not Found" << std::endl;
    } else {
        std::cout << "Ceil=" << ceil << " Found " << *found_it << std::endl;
    }
}
