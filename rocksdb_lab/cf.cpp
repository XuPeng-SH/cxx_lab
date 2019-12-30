#include "cf.h"
#include <iostream>
#include <rocksdb/db.h>
#include <vector>
#include <string>

using namespace rocksdb;
using namespace std;

void column_family_demo() {
    Options options;
    options.create_if_missing = true;
    auto path = "/tmp/cf_demo";

    std::vector<string> cf_names;
    std::vector<ColumnFamilyDescriptor> column_families;
    std::vector<ColumnFamilyHandle*> handles;

    DB::ListColumnFamilies(DBOptions(), path, &cf_names);

    DB *db;
    Status s;
    if (cf_names.size() == 0) {
        s = DB::Open(options, path, &db);
    } else {
        for (auto& name : cf_names) {
            column_families.push_back(ColumnFamilyDescriptor(name, ColumnFamilyOptions()));
        }
        s = DB::Open(options, path, column_families, &handles, &db);
    }

    cout << s.ToString() << endl;
    assert(s.ok());

    for (auto handle : handles) {
        cout << handle->GetName() << endl;
        cout << handle->GetID() << endl;
    }

    cout << "====================================" << endl;

    // Add CF extra
    ColumnFamilyOptions cf_options(options);
    ColumnFamilyHandle* handle;
    s = db->CreateColumnFamily(cf_options, "extra", &handle);
    cout << s.ToString() << endl;

    if (s.ok())
        handles.push_back(handle);

    for (auto handle : handles) {
        cout << handle->GetName() << endl;
        cout << handle->GetID() << endl;
    }

    // Delete CF extra
    handle = handles.back();
    handles.pop_back();
    s = db->DropColumnFamily(handle);
    cout << s.ToString() << endl;
    assert(s.ok());
    delete handle;

    for (auto handle : handles) {
        delete handle;
    }
    delete db;
}
