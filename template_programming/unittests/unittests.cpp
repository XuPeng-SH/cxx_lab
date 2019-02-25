#include <iostream>
#include "my_templates.hpp"

#include <gtest/gtest.h>


DEFINE_HAS_CLASS_TYPE(second_argument_type);

class MyUnitTest : public ::testing::Test {

};

TEST_F(MyUnitTest, is_pointer) {
    typedef const char* StrT;
    typedef const char ConstCharT;
    bool is_a_pointer = is_pointer<StrT>::value;
    EXPECT_EQ(is_a_pointer, true);
    is_a_pointer = is_pointer<ConstCharT>::value;
    EXPECT_EQ(is_a_pointer, false);
};

TEST_F(MyUnitTest, MyBinder2nd) {
    MyBinder2nd<std::less<int>, int> binder(std::less<int>(), 10);
    bool has = HAS_CLASS_TYPE(std::less<int>, second_argument_type);
    EXPECT_EQ(has, true);

    typedef MyBinder2nd<std::less<int>,int> MyBinder2ndType;
    has = HAS_CLASS_TYPE(MyBinder2ndType, second_argument_type);
    EXPECT_EQ(has, false);

    has = FunctionChecker<void ()>::has_first_argument;
    EXPECT_EQ(has, false);

    has = FunctionChecker<std::less<int>>::has_first_argument;
    EXPECT_EQ(has, true);

    has = FunctionChecker<std::less<int>>::has_second_argument;
    EXPECT_EQ(has, true);

    has = FunctionChecker<MyBinder2nd<std::less<int>, int>>::has_second_argument;
    EXPECT_EQ(has, false);
}
