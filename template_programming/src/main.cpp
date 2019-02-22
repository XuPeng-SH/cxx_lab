#include <iostream>
#include "container.hpp"
#include "my_templates_demo.hpp"
#include "common.hpp"

using namespace std;

void cc_demo(int a) {
    std::cout << a << std::endl;
}

int main(int argc, char** argv) {
#ifdef V2
    find_if_demo();
#endif
    CALL_DEMO(MyBinder2nd);
    CALL_DEMO(FunctionChecker);
#ifdef V3
    CALL_DEMO(IsPointer);
#endif
    return 0;
}
