#include "labs.h"

#include <string>
#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

void multi_index_lab() {
    /* using StringContainer = boost::multi_index_container<std::string>; */
    using StringContainer = boost::multi_index_container<std::string,
          boost::multi_index::indexed_by<
            boost::multi_index::ordered_unique<boost::multi_index::identity<std::string>>
          >
    >;
    StringContainer employees;
    employees.insert("xp");
    employees.insert("nana");
    employees.insert("juan");
    employees.insert("wang");

    for (auto& e : employees) {
        std::cout << e << std::endl;
    }

    auto lb = employees.lower_bound("k");
    auto ub = employees.lower_bound("y");

    while (lb != ub) {
        std::cout << "range " <<  *lb << std::endl;
        ++lb;
    }

}
