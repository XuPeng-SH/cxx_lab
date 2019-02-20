#include "my_templates_demo.hpp"
#include <iostream>
#include <string>
#include <functional>
#include <algorithm>


DEFINE_HAS_CLASS_TYPE(second_argument_type);


void no_argument() {}

START_DEMO(FunctionChecker)
    MyBinder2nd<std::less<int>, int> binder(std::less<int>(), 10);
    std::cout << HAS_CLASS_TYPE(std::less<int>, second_argument_type) << std::endl;
    typedef MyBinder2nd<std::less<int>,int> MyBinder2ndType;
    std::cout << HAS_CLASS_TYPE(MyBinder2ndType, second_argument_type) << std::endl;

    std::cout << std::boolalpha << FunctionChecker<void ()>::has_first_argument << std::endl;
    std::cout << std::boolalpha << FunctionChecker<std::less<int>>::has_first_argument << std::endl;
    std::cout << std::boolalpha << FunctionChecker<std::less<int>>::has_second_argument << std::endl;
    std::cout << std::boolalpha << FunctionChecker<MyBinder2nd<std::less<int>, int>>::has_second_argument << std::endl;
END_DEMO;

START_DEMO(MyBinder2nd)
    int second = 18;
    MyBinder2nd<std::less<int>, int> binder(std::less<int>(), second);

    std::cout << binder(13) << std::endl;
    std::cout << binder(19) << std::endl;
END_DEMO;
