#include <iostream>
#include <boost/bind.hpp>

using namespace std;

void aaa(int a) {
    std::cout << a << std::endl;
}

int main(int argc, char** argv) {
    cout << "hello adt lab" << endl;

    boost::bind(&aaa, 10)();

    return 0;
}
