#include <iostream>
#include "Utils.h"
#include "Resources.h"

using namespace std;

class Resource : public ReferenceProxy {
public:
    void OnDeRefCallBack() {
        cout << "OnDeRefCallBack" << endl;
    }

    Resource() {
        RegisterOnNoRefCB(std::bind(&Resource::OnDeRefCallBack, this));
    }

    int count;

};

using ResourcePtr = std::shared_ptr<Resource>;

int main() {
    auto cb = []() -> void {
        cout << "Call CB" << endl;
    };
    {
        auto res = std::make_shared<Resource>();
        {
            auto scoped = std::make_shared<ScopedResource>(res);
        }
    }

    Collection c1(1, "xx");
    cout << c1.ToString() << endl;
    return 0;
}
