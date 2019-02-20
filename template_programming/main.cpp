#include <iostream>
#include "container.hpp"
#include "my_templates_demo.hpp"
#include "common.hpp"

using namespace std;

void cc_demo(int a) {
    std::cout << a << std::endl;
}

int main(int argc, char** argv) {
    find_if_demo();
    CALL_DEMO(MyBinder2nd);
    CALL_DEMO(FunctionChecker);
    return 0;
}
