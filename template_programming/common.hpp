#pragma once

#include <iostream>
#include <string>
#include <vector>

#define DEMO_NAME(name) name##_demo
#define CONTACT(LHS, RHS) LHS##RHS
#define CALL_DEMO(name, ...) DEMO_NAME(name)(__VA_ARGS__);

#define START_DEMO(name) void DEMO_NAME(name) () { \
    std::string gDemoName = #name;  \
    std::cout << "\033[1;32m------[DEMO]" << gDemoName << " Started-----------\033[0m" << std::endl;

#define END_DEMO \
    std::cout << "\033[1;32m------[DEMO]" << gDemoName << " Ended-------------\033[0m\n" << std::endl; \
}
