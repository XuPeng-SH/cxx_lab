#include <iostream>
#include <gflags/gflags.h>
#include <iostream>
#include <memory>

#include "Port.h"

using namespace std;


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    int ret = 0;
    Data data;
    data.val = 999;

    auto uint = State::getUint(&data);
    cout << "uint = " << std::hex << uint << endl;
    cout << "addr = " << (void*)(&data) << endl;
    auto ptr = State::getPtr(uint);
    cout << "ptr.val = " << std::dec << ptr->val << endl;

    {
        cout << "FLAGS_MASK = " << std::hex << State::FLAGS_MASK << endl;
        cout << "PTR_MASK   = " << std::hex << State::PTR_MASK << std::dec << endl;
    }

    /* std::vector<std::shared_ptr<State::DataPtr>> vec; */
    /* for (auto i = 0; i < 10; ++i ) { */
    /*     auto dptr = std::make_shared<State::DataPtr>(); */
    /*     cout << "uint of dptr->data_ = " << std::hex << State::getUint(dptr->data_) << endl; */
    /* } */

    {
        auto d1 = State::DataPtr();
        d1.data_->val = 11;
        cout << "uint of d1.data_ = " << std::hex << "0x" << State::getUint(d1.data_) << endl;
        std::atomic<Data*> adata(new Data());
        adata.load()->val = 22;
        cout << "uint of adata->val = " << std::hex << "0x" << State::getUint(adata.load()) << endl;

        auto uuint = d1.Swap(adata, State::HAS_DATA, State::HAS_DATA);
        cout << "uuint 0x" << uuint << endl;

        cout << "xxx uint of d1.data_ = " << std::hex << "0x" << State::getUint(d1.data_) << endl;
        cout << "xxx uint of adata->val = " << std::hex << "0x" << State::getUint(adata.load()) << endl;

        /* delete adata.load(); */
    }

    {
        auto s1 = State();
        cout << "s1.data_ " << State::getUint(s1.data_) << endl;
        auto pre = s1.SetFlags(State::HAS_DATA, State::HAS_DATA);
        cout << "pre is " << pre << endl;

        auto post = s1.GetFlags();
        cout << "post is " << post << endl;
    }

    {
        std::atomic<int> aval = 1;
        int expected = 2;
        int desired = 3;

        while (!aval.compare_exchange_weak(expected, 3));
        cout << "aval = " << aval << endl;
        cout << "expected = " << expected << endl;
        cout << "desired = " << desired << endl;
    }

    return ret;
}
