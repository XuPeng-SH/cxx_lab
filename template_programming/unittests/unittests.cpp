#include <iostream>
#include "my_templates.hpp"

#include <gtest/gtest.h>

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
