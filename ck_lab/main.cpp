#include <iostream>
#include <gflags/gflags.h>
#include <iostream>
#include <memory>

#include "Port.h"
#include "Graph.h"
#include "forwards.h"
/* #include "HierarchyGuard.h" */

using namespace std;
using namespace MyDB;


int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::mutex m;
    std::unique_lock<std::mutex> guard_lock(m);
    DatabaseGuard::HighMutexPtr high_mutex;
    DatabaseGuard::RelationPtr rel = std::make_shared<DatabaseGuard::Relation>();

    DatabaseGuard guard(high_mutex, "", rel, "", std::move(guard_lock));
    /* DatabaseGuard guard(high_mutex, "", rel, "", std::move(guard_lock)); */
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
