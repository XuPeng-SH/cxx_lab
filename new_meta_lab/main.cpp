#include <iostream>
#include "Utils.h"
#include "Resources.h"
#include "Snapshots.h"
#include "Proxy.h"

using namespace std;

class Resource : public ReferenceProxy {
public:
    using Ptr = std::shared_ptr<Resource>;
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
            using ResourceT = ScopedResource<Resource>;
            auto scoped = std::make_shared<ResourceT>(res);
        }
    }

    /* auto& collections_holder = CollectionsHolder::GetInstance(); */
    /* collections_holder.Dump("-----"); */
    /* auto c1 = collections_holder.GetResource(4); */
    /* collections_holder.Dump("111111"); */

    /* cout << c1->Get()->RefCnt() << endl; */
    /* c1->Get()->Ref(); */
    /* cout << c1->Get()->RefCnt() << endl; */
    /* c1->Get()->Ref(); */
    /* cout << c1->Get()->RefCnt() << endl; */
    /* c1->Get()->UnRef(); */
    /* cout << c1->Get()->RefCnt() << endl; */
    /* c1->Get()->UnRef(); */
    /* cout << c1->Get()->RefCnt() << endl; */


    /* /1* for (auto i=0; i<100; ++i) { *1/ */
    /* /1*     collections_holder.GetResource(i); *1/ */
    /* /1* } *1/ */

    /* collections_holder.Dump(); */
    /* c1->Get()->UnRef(); */
    /* cout << c1->Get()->RefCnt() << endl; */
    /* collections_holder.Dump(); */

    SnapshotsHolder ss_holder;
    ss_holder.Add(1);
    ss_holder.Add(2);

    return 0;
}
