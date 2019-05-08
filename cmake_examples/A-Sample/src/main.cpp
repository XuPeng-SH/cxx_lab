#include "hello.hpp"
#include <iostream>

using namespace std;

typename std::aligned_storage<sizeof(int), alignof(int)>::type IntStorage;

int main(int argc, char** argv) {
    /* say_hello(); */
    cout << sizeof(IntStorage) << endl;
    cout << alignof(IntStorage) << endl;
    cout << alignof(int) << endl;
    static_assert(sizeof(IntStorage) <= sizeof(int), "Error");
    return 0;
}
