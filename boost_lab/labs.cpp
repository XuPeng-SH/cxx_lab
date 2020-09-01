#include "labs.h"

#include <string>
#include <iostream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
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

struct Employee {
    int id;
    std::string name;

    bool operator<(const Employee& o) const {
        return id < o.id;
    }
};

void complex_multi_index_lab() {
    using EmployeeContainer = boost::multi_index_container<Employee,
          boost::multi_index::indexed_by<
              boost::multi_index::ordered_unique<boost::multi_index::identity<Employee>>,
              boost::multi_index::ordered_non_unique<boost::multi_index::member<Employee, std::string, &Employee::name>>
          >
    >;

    EmployeeContainer employees;
    employees.insert({2, "xp"});
    employees.insert({1, "zh"});
    employees.insert({3, "na"});
    employees.insert({4, "oh"});

    for (auto& e : employees) {
        std::cout << "complex rank id " << e.id << " " << e.name << std::endl;
    }

    const EmployeeContainer::nth_index<1>::type& name_index = employees.get<1>();
    for (auto& it : name_index) {
        std::cout << "complex rank name " << it.id << " " << it.name << std::endl;
    }
}

struct WordCnt {
    std::string word;
    int cnt;

    WordCnt(const std::string& w, int c) : word(w), cnt(c) {}

    bool operator<(const WordCnt& o) const {
        return cnt < o.cnt;
    }
};

void hash_multi_index_lab() {
    using WordCntContainer = boost::multi_index_container<WordCnt,
          boost::multi_index::indexed_by<
            boost::multi_index::ordered_non_unique<boost::multi_index::identity<WordCnt>>,
            boost::multi_index::hashed_unique<boost::multi_index::member<WordCnt, std::string, &WordCnt::word>>
          >
    >;

    WordCntContainer container;
    std::vector<std::string> contents;

    boost::split(contents, "hello hello hello hello hello cpp cpp cpp cpp go go go python python shell",
            boost::is_any_of(" "));

    auto& word_idx = container.get<1>();

    for (auto& word : contents) {
        auto iter = word_idx.find(word);
        if (iter == word_idx.end()) {
            word_idx.insert({word, 1});
        } else {
            word_idx.modify(iter, [](WordCnt& wc) {
                wc.cnt++;
            });
        }
    }

    for (const auto wc : container) {
        std::cout << "wc " << wc.word << " " << wc.cnt << std::endl;
    }

    for (auto& wc : word_idx) {
        std::cout << "sec wc " << wc.word << " " << wc.cnt << std::endl;
    }


}
