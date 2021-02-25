#include <iostream>
#include <gflags/gflags.h>
#include <iostream>
#include <memory>

#include "Port.h"
#include "Graph.h"
#include "HierarchyGuard.h"

using namespace std;
using namespace MyDB;


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::unique_lock<std::mutex> guards_lock;
    StrIDGuard::HighMutexPtr high_mutex;
    StrIDGuard::RelationPtr rel = std::make_shared<StrIDGuard::Relation>();

    StrIDGuard guard(high_mutex, "", rel, "");
    /* StrIDGuard guard(high_mutex, "", rel, "", std::move(guards_lock)); */
    cout << guard.GetEntryCount("") << endl;

    /* shared_ptr<int> a = nullptr; */
    /* shared_ptr<int> b = std::make_shared<int>(2); */

    /* if (a) { */
    /*     cout << "pre a=" << *a << " b is nullptr"; */
    /* } else { */
    /*     cout << "pre b=" << *b << " a is nullptr"; */
    /* } */
    /* a.swap(b); */
    /* if (a) { */
    /*     cout << "post a=" << *a << " b is nullptr"; */
    /* } else { */
    /*     cout << "post b=" << *b << " a is nullptr"; */
    /* } */
    /* a.swap(b); */

    return 0;
}
