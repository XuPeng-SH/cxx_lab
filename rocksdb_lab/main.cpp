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
DEFINE_string(prefix, "", "mock data prefix");
DEFINE_bool(search, false, "search data");
DEFINE_int32(nq, 100, "n query");

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
    }
    cout << "search: " << (FLAGS_search ? "true" : "false") << "\n";
    if (FLAGS_search) {
        cout << "\tnq: " << FLAGS_nq << "\n";
    }
    cout << "======== Config Block End   =================\n" << endl;

    map<string, DB*> db_map;
    make_segments(FLAGS_path, FLAGS_segments, db_map);

    if (FLAGS_mock)
        mock_segments_data(db_map, FLAGS_nb, FLAGS_prefix, FLAGS_async);

    if (FLAGS_search)
        search_segments(db_map, FLAGS_nq, FLAGS_prefix, FLAGS_async);

    destroy_segments(db_map);

    return 0;
}
