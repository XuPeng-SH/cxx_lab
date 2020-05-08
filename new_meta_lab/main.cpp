#include <iostream>
#include "Utils.h"
#include "Resources.h"
#include "Snapshots.h"

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

    CollectionPtr c1 = std::make_shared<Collection>(1, "xx");
    cout << c1->ToString() << endl;
    auto& holder = CollectionsHolder::GetInstance();
    holder.Dump("1");
    holder.Add(c1);
    holder.Dump("2");
    holder.Remove(c1->GetID());
    holder.Dump("3");

    MappingT mappings = {1,2,3,4};
    auto c_c = std::make_shared<CollectionCommit>(1, 1, mappings);
    cout << c_c->ToString() << endl;

    auto& c_c_holder = CollectionCommitsHolder::GetInstance();
    c_c_holder.Add(c_c);

    auto ss = std::make_shared<Snapshot>(1);
    c_c_holder.Dump("4");

    /* auto snapshot = Manager.GetSnapshot(collection_name); */
    /* Manager.ReleaseSnapshot(snapshot); */

    SnapshotsHolder ss_holder;
    ss_holder.Add(1);

    return 0;
}
