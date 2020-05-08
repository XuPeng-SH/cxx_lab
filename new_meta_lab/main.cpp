#include <iostream>
#include "Utils.h"

using namespace std;

class Resource : public ReferenceProxy {
public:
    void OnDeRefCallBack() override {
        cout << "OnDeRefCallBack" << endl;
    }

    int count;

};

int main() {
    auto cb = []() -> bool {
        cout << "Call CB" << endl;
        return true;
    };
    {
        Resource res;
        res.RegisterOnDeRefCB(cb);
        res.Ref();
        res.UnRef();
    }
    return 0;
}
