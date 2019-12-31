#include <string>
#include <iostream>
#include <rocksdb/db.h>
#include <rocksdb/status.h>
#include <map>
#include <gflags/gflags.h>
#include "db_op.h"
#include "cf.h"

using namespace std;
using namespace rocksdb;

DEFINE_bool(async, true, "write or get segments in multithread context");
DEFINE_string(path, "/home/xupeng/data/ldb/", "segements base path");
DEFINE_int32(segments, 100, "segements num");
DEFINE_bool(mock, false, "mock segments data");
DEFINE_int32(nb, 10000, "mock data per segment");
DEFINE_string(mcf, "default", "specify cf name for mock");
DEFINE_string(prefix, "", "mock data prefix");
DEFINE_bool(search, false, "search data");
DEFINE_int32(nq, 100, "n query");
DEFINE_string(scf, "default", "specify cf name for search");

int main(int argc, char** argv) {
    /* column_family_demo(); */
    /* return 0; */
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    cout << "\n======== Config Block Start =================\n";
    cout << "path: " << FLAGS_path << "\n";
    cout << "async: " << (FLAGS_async ? "true" : "false") << "\n";
    cout << "segments: " << FLAGS_segments << "\n";
    cout << "mock: " << (FLAGS_mock ? "true" : "false") << "\n";
    if (FLAGS_mock) {
        cout << "\tnb: " << FLAGS_nb << "\n";
        cout << "\tprefix: " << FLAGS_prefix << "\n";
        cout << "\tmcf: " << FLAGS_mcf << "\n";
    }
    cout << "search: " << (FLAGS_search ? "true" : "false") << "\n";
    if (FLAGS_search) {
        cout << "\tnq: " << FLAGS_nq << "\n";
        cout << "\tscf: " << FLAGS_scf << "\n";
    }
    cout << "======== Config Block End   =================\n" << endl;

    map<string, DB*> db_map;
    HandleMapT handles_map;
    make_segments(FLAGS_path, FLAGS_segments, db_map, handles_map);

    if (FLAGS_mock) {
        HandleSelectorT selector;
        for (auto& kv: handles_map) {
            bool found = false;
            for (auto& handle : kv.second) {
                /* cout << "handle_name=" << handle->GetName() << endl; */
                if (handle->GetName() == FLAGS_mcf) {
                    selector[kv.first] = handle;
                    found = true;
                    break;
                }
            }

            if (!found) {
                ColumnFamilyOptions cf_options;
                ColumnFamilyHandle* handle;
                auto s = db_map[kv.first]->CreateColumnFamily(cf_options, FLAGS_mcf, &handle);

                if (s.ok()) {
                    kv.second.push_back(handle);
                    selector[kv.first] = handle;
                } else {
                    cout << s.ToString() << endl;
                }

            }
        }
        mock_segments_data(db_map, selector, FLAGS_nb, FLAGS_prefix, FLAGS_async);
    }

    if (FLAGS_search) {
        bool ret = false;
        HandleSelectorT selector;
        for (auto& kv: handles_map) {
            map<string, string> dict;
            for (auto& handle : kv.second) {
                if (handle->GetName() == FLAGS_scf) {
                    selector[kv.first] = handle;
                    break;
                }
            }
            if (selector.find(kv.first) == selector.end()) {
                cout << "Cannot find CF " << kv.first << endl;
                ret = true;
                break;
            }
            load_all(db_map[kv.first], selector[kv.first], dict);
        }
        if (!ret)
            search_segments(db_map, selector, FLAGS_nq, FLAGS_prefix, FLAGS_async);
    }

    destroy_segments(db_map, handles_map);

    return 0;
}
