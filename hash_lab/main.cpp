#include <string>
#include <iostream>
#include <boost/functional/hash.hpp>

struct Student {
    std::string name_;
    int id_;
    static constexpr size_t SEED = 1;

    explicit Student(const std::string& name, int id) : name_(name), id_(id) {}
    size_t
    HashCode() const {
        size_t seed = SEED;
        boost::hash_combine(seed, name_);
        boost::hash_combine(seed, id_);
        return seed;
    }
};

int main() {
    auto st = Student("XP", 1);
    std::cout << st.HashCode() << std::endl;
    return 0;
}
