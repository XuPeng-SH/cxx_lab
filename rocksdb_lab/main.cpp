#include <string>
#include <iostream>
#include <rocksdb/db.h>
#include <rocksdb/status.h>
#include <map>
#include <gflags/gflags.h>
#include <chrono>
#include <thread>
#include "db_op.h"
#include "cf.h"
#include "rocksdb_impl.h"
#include "rocksdb_util.h"
#include <rocksdb/comparator.h>
#include <rocksdb/table.h>
#include "database.h"
#include "doc.h"
#include "utils.h"

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
DEFINE_bool(rall, false, "read all");
DEFINE_bool(print, false, "do print");

DEFINE_string(tname, "default", "table name");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    /* int a1=257; */
    /* int a2 = 8; */
    /* string b1, b2; */
    /* db::Serializer::SerializeNumeric(a1, b1); */
    /* db::Serializer::SerializeNumeric(a2, b2); */

    /* std::cout << (b1 > b2) << std::endl; */
    /* return 0; */

    auto options = db::DefaultOpenOptions();

    // Setup threadpool for compaction and flush
    auto env = rocksdb::Env::Default();
    env->SetBackgroundThreads(2, rocksdb::Env::LOW);
    env->SetBackgroundThreads(1, rocksdb::Env::HIGH);
    options->env = env;
    options->max_background_compactions = 2;
    options->max_background_flushes = 1;

    // Set Comparator
    /* db::MyComparator cmp; */
    /* options->comparator = &cmp; */

    // Set Block Cache
    size_t capacity = (size_t)5 * 1024 * 1024 * 1024;
    std::shared_ptr<Cache> cache = rocksdb::NewLRUCache(capacity);
    rocksdb::BlockBasedTableOptions table_options;
    table_options.block_cache = cache;
    table_options.block_size = 32 * 1024;
    options->table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));

    // Set write buffer size
    options->write_buffer_size = 256 << 10;

    rocksdb::DB *kvdb;
    rocksdb::DB::Open(*options, FLAGS_path, &kvdb);
    std::shared_ptr<rocksdb::DB> skvdb(kvdb);

    auto TEST_STR_TO_UINT64 = [&]() {
        auto start = chrono::high_resolution_clock::now();
        db::demo::check_str_to_uint64();
        auto end = chrono::high_resolution_clock::now();
        cout << "check_str_to_uint64 takes " << chrono::duration<double, std::milli>(end-start).count() << endl;
        return 0;
    };

    if (FLAGS_mock){
        db::demo::mock_uid_id_mapping(skvdb, FLAGS_nb);
        return 0;
    }


    auto impl = std::make_shared<db::RocksDBImpl>(skvdb);
    auto thisdb = std::make_shared<db::MyDB>(impl);
    auto schema = std::make_shared<DocSchema>();
    schema->AddLongField(LongField("_id"));

    StringField uid_field("uid");
    LongField age_field("age");

    uid_field.SetMaxLength(20);
    uid_field.SetMinLength(10);

    schema->AddLongField(age_field)
           .AddStringField(uid_field)
           .Build();

    auto CHECK_SERIALIZER = [&]()
    {
        std::string meta;
        Serializer::SerializeFieldMeta(uid_field, meta);
        uint8_t type;
        std::string fname;
        Serializer::DeserializeFieldMeta(meta, type, fname);
        std::string doc_serialized;
        schema->Serialize(doc_serialized);
        DocSchema dschema;
        DocSchema::Deserialize(doc_serialized, dschema);
        /* return 0; */
    };

    auto ADD_DOC = [&](int num)
    {
        std::string table_name = "mockt";
        thisdb->CreateTable(table_name, *schema);
        for (auto i=0; i<num; i++) {
            Doc mydoc(Helper::NewPK(i+10000), schema);
            mydoc.AddLongFieldValue("age", 10+i)
                 .AddStringFieldValue("uid", std::to_string(1000000+i))
                 .Build();

            auto s = thisdb->AddDoc(table_name, mydoc);
            if (!s.ok()) {
                /* db::demo::read_all(skvdb, nullptr, true); */
                std::cout << s.ToString() << " " << __func__ << ":" << __LINE__ << std::endl;
                return 0;
            }
        }
    };

    ADD_DOC(FLAGS_nb);

    std::vector<std::string> vec;
    std::stringstream ss;
    for (auto i=0; i<FLAGS_nb; ++i) {
        ss << FLAGS_prefix << i;
        vec.push_back(ss.str());
        ss.str("");
    }

    auto MT_CREATE_TABLE = [&](int tnum) {
        std::vector<std::pair<size_t, size_t>> range;
        size_t step = vec.size()/tnum;

        for (size_t i=0; i<tnum; i++) {
            auto start = i * step;
            auto end = (i+1) * step;
            if (i+1 == tnum) {
                end = vec.size();
            }
            /* std::cout << "start=" << start << " end=" << end <<std::endl; */
            range.push_back({start, end});
        }

        std::vector<std::thread> tvec;

        auto start = chrono::high_resolution_clock::now();
        for (auto& itrange : range) {
            tvec.push_back(std::thread([&](const std::pair<size_t, size_t>& r) {
                auto start = std::get<0>(r);
                auto end = std::get<1>(r);
                rocksdb::Status s;
                for(auto i=start; i<end; ++i) {
                    s = thisdb->CreateTable(vec[i], *schema);
                    if (!s.ok()) {
                        std::cout << "Error: " << s.ToString() << std::endl;
                        break;
                    }
                }
            }, itrange));
        }

        for (auto& t: tvec) {
            t.join();
        }
        auto end = chrono::high_resolution_clock::now();
        cout << __FILE__ << ":" << __LINE__ << " tnum=" << tnum << " write takes " << chrono::duration<double, std::milli>(end-start).count() << endl;
    };
    /* MT_CREATE_TABLE(1); */

    auto READ_ALL = [&](bool do_print) {
        auto start = chrono::high_resolution_clock::now();
        /* db::demo::read_all(thisdb, opt, do_print); */
        thisdb->Dump(do_print);
        auto end = chrono::high_resolution_clock::now();
        cout << "readall takes " << chrono::duration<double, std::milli>(end-start).count() << endl;
    };

    if (FLAGS_rall)
        READ_ALL(FLAGS_print);

    {
        std::shared_ptr<Doc> doc;
        auto s = thisdb->GetDoc("mockt", 10000, doc);
        cout << s.ToString() << endl;
    }
    {
        std::vector<std::shared_ptr<Doc>> docs;
        db::FieldsFilter filters;
        db::FieldFilter filter;
        long age_upper = 12;
        long age_lower = 0;
        Serializer::SerializeNumeric(age_upper, filter.upper_bound);
        Serializer::SerializeNumeric(age_lower, filter.lower_bound);
        filter.name = "age";
        filters.push_back(std::move(filter));
        auto s = thisdb->GetDocs("mockt", docs, filters);
        cout << s.ToString() << endl;
    }

    auto READ_WITH_UPPER_LOWER = [&](bool do_print) {
        rocksdb::ReadOptions rdopts;
        std::string upper(db::DBTableUidIdMappingPrefix);
        std::string lower(db::DBTableUidIdMappingPrefix);
        uint64_t tid = 0;
        /* uint64_t l_uid = 950079711; */
        uint64_t l_uid = 265167685;
        uint64_t u_uid = 1549469410 + 1;
        upper.append((char*)&tid, sizeof(tid));
        upper.append((char*)&u_uid, sizeof(u_uid));
        lower.append((char*)&tid, sizeof(tid));
        lower.append((char*)&l_uid, sizeof(l_uid));

        Slice l(lower);
        Slice u(upper);

        rdopts.iterate_lower_bound = &l;
        rdopts.iterate_upper_bound = &u;
        rdopts.readahead_size = 1024*512;

        auto start = chrono::high_resolution_clock::now();
        /* db::demo::read_all(thisdb, &rdopts, true); */
        thisdb->Dump(true);
        auto end = chrono::high_resolution_clock::now();
        cout << "readpartial takes " << chrono::duration<double, std::milli>(end-start).count() << endl;
    };

    /* test_read_with_upper_lower(true); */

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
