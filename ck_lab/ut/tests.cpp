#include <memory>
#include <gtest/gtest.h>
#include <iostream>
#include <atomic>

#include "IProcessor.h"
#include "Graph.h"
#include "Port.h"

using namespace std;
using namespace MyDB;

class MyUT : public ::testing::Test {
};

TEST_F(MyUT, Atomic) {
    {
        int old_aval = 1;
        std::atomic<int> aval = old_aval;
        int expected = 2;
        int desired = 3;

        while (!aval.compare_exchange_weak(expected, desired));
        ASSERT_EQ(aval, desired);
        ASSERT_EQ(expected, old_aval);
    }
}

TEST_F(MyUT, Port_State) {
    {
        Data data;
        data.val = 999;
        auto uint = State::GetUint(&data);
        auto ptr = State::GetPtr(uint);
        ASSERT_EQ(ptr->val, data.val);

    }
    {
        auto d1 = State::DataPtr();
        d1.data_->val = 11;
        std::atomic<Data*> adata(new Data());
        adata.load()->val = 22;
        auto uuint = d1.Swap(adata, State::HAS_DATA, State::HAS_DATA);
        ASSERT_EQ(uuint, 0x0);
        ASSERT_EQ(State::GetUint(d1.data_) & State::FLAGS_MASK, 0);
        ASSERT_EQ(State::GetUint(adata.load()) & State::FLAGS_MASK, State::HAS_DATA);

        uuint = d1.Swap(adata, State::IS_NEEDED, 0);
        ASSERT_EQ(State::GetUint(d1.data_) & State::FLAGS_MASK, 0);
        ASSERT_EQ(uuint, State::HAS_DATA);
        ASSERT_EQ(State::GetUint(adata.load()) & State::FLAGS_MASK, State::HAS_DATA | State::IS_NEEDED);

        uuint = d1.Swap(adata, State::IS_FINISHED, State::IS_NEEDED | State::HAS_DATA);
        ASSERT_EQ(State::GetUint(d1.data_) & State::FLAGS_MASK, 0);
        ASSERT_EQ(uuint, State::HAS_DATA | State::IS_NEEDED);
        ASSERT_EQ(State::GetUint(adata.load()) & State::FLAGS_MASK, State::IS_FINISHED);
    }
}
