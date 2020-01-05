#include <string>
#include <iostream>
#include <rocksdb/db.h>
#include <rocksdb/status.h>
#include <map>
#include <gflags/gflags.h>
#include <chrono>
#include "db_op.h"
#include "cf.h"
#include "rocksdb_impl.h"
#include "rocksdb_util.h"
#include "doc.h"

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

DEFINE_string(tname, "default", "table name");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    auto options = db::DefaultOpenOptions();
    rocksdb::DB *kvdb;
    rocksdb::DB::Open(*options, FLAGS_path, &kvdb);
    std::shared_ptr<rocksdb::DB> skvdb(kvdb);

    db::demo::mock_uid_id_mapping(skvdb);

    auto impl = std::make_shared<db::RocksDBImpl>(skvdb);
    auto thisdb = std::make_shared<db::MyDB>(impl);
    auto schema = std::make_shared<DocSchema>();

    StringField uid_field("uid");
    FloatVectorField vec_field("vec");

    uid_field.SetMaxLength(20);
    uid_field.SetMinLength(10);

    vec_field.SetMaxLength(128);
    vec_field.SetMinLength(128);

    schema->AddStringField(uid_field)
           .AddFloatVectorField(vec_field)
           .Build();

    std::cout << schema->Dump() << std::endl;

    std::vector<std::string> vec;
    std::stringstream ss;
    for (auto i=0; i<FLAGS_nb; ++i) {
        ss << FLAGS_prefix << i;
        vec.push_back(ss.str());
        ss.str("");
    }
    std::cout << "Starting ..." << std::endl;
    for (auto& v : vec) {
        thisdb->CreateTable(v, *schema);
    }

    auto start = chrono::high_resolution_clock::now();
    /* db::demo::read_all(skvdb, nullptr, false); */
    auto end = chrono::high_resolution_clock::now();
    cout << "readall takes " << chrono::duration<double, std::milli>(end-start).count() << endl;

    rocksdb::ReadOptions rdopts;
    std::string upper(db::DBTableUidIdMappingPrefix);
    std::string lower(db::DBTableUidIdMappingPrefix);
    uint64_t tid = 0;
    uint64_t l_uid = 612856698-1;
    uint64_t u_uid = 1392120040 + 1;
    upper.append((char*)&tid, sizeof(tid));
    upper.append((char*)&u_uid, sizeof(u_uid));
    lower.append((char*)&tid, sizeof(tid));
    lower.append((char*)&l_uid, sizeof(l_uid));

    Slice l(lower);
    Slice u(upper);

    rdopts.iterate_lower_bound = &l;
    rdopts.iterate_upper_bound = &u;
    std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
    start = chrono::high_resolution_clock::now();
    db::demo::read_all(skvdb, &rdopts, false);
    end = chrono::high_resolution_clock::now();
    cout << "readall takes " << chrono::duration<double, std::milli>(end-start).count() << endl;

    start = chrono::high_resolution_clock::now();
    db::demo::check_str_to_uint64();
    end = chrono::high_resolution_clock::now();
    cout << "check_str_to_uint64 takes " << chrono::duration<double, std::milli>(end-start).count() << endl;

    return 0;
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
