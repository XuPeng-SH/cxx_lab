#include <string>
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/status.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <chrono>
#include <thread>
#include <map>
#include <type_traits>

#include "fields.h"
#include "doc.h"

using namespace std;
using namespace leveldb;

void mock_data(DB* db, size_t num, string pre="") {
  cout << __func__ << " num=" << num << " pre=" << pre << endl;

  auto write_options = WriteOptions();

  string key_prefix = pre + "mock_prefix_user_key:";
  string val_prefix = "mock_prefix_user_value:";
  auto start = chrono::high_resolution_clock::now();
  for (auto i=0; i<num; ++i) {
      auto strnum = to_string(i);
      db->Put(write_options, key_prefix + strnum, val_prefix + strnum);
      if (i % 5000000 == 0) {
          cout << "handling the " << i << " th" << endl;
      }
  }
  auto end = chrono::high_resolution_clock::now();
  cout << __func__ << " takes " << chrono::duration<double, std::milli>(end-start).count() << endl;
}

void load_all(DB* db, map<string, string>& dict) {
    size_t count = 0;
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    dict.clear();
    auto start = chrono::high_resolution_clock::now();
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        ++count;
        dict[it->key().ToString()] = it->value().ToString();
    }
    cout << count << " loaded" << endl;
    auto end = chrono::high_resolution_clock::now();
    cout << __func__ << " takes " << chrono::duration<double, std::milli>(end-start).count() << " ms" << endl;
    delete it;
}

void search_seg(DB* db, size_t n, string pre="") {
    bool debug = false;
    auto rop = ReadOptions();
    auto start = chrono::high_resolution_clock::now();
    string value;
    string key_prefix = pre + "mock_prefix_user_key:";
    for (auto i=0; i<n; ++i) {
        auto key = key_prefix + to_string(i);
        db->Get(rop, key, &value);
        if (debug)
            cout << "key, value = " << key << ", " << value << endl;
    }
    cout << n << " checked" << endl;
    auto end = chrono::high_resolution_clock::now();
    cout << __func__ << " takes " << chrono::duration<double, std::milli>(end-start).count() << " ms" << endl;
}

void make_segments(const string& db_path_prefix, size_t n, map<string, DB*>& db_map) {
    Options options;
    options.create_if_missing = true;
    for (auto i=0; i<n; ++i) {
        auto segment_name = db_path_prefix+"segment_"+to_string(i);
        DB *db;
        DB::Open(options, segment_name, &db);
        db_map[segment_name] = db;
    }
}

void destroy_segments(map<string, DB*>& db_map) {
    for (auto& kv: db_map) {
        /* cout << kv.first << endl; */
        delete kv.second;
    }
}

void tt(void* value) {
    cout << *(int*)value << endl;
}

int main(int argc, char** argv) {
    auto schema = std::make_shared<DocSchema>();

    StringField uid_field("uid");
    FloatVectorField vec_field("vec");

    uid_field.SetMaxLength(20);
    uid_field.SetMinLength(10);

    vec_field.SetMaxLength(4);
    vec_field.SetMinLength(4);

    schema->AddLongField(LongField("age"))
           .AddLongField(LongField("likes"))
           .AddFloatField(FloatField("score"))
           .AddStringField(uid_field)
           .AddFloatVectorField(vec_field)
           .Build();
    cout << schema->Dump() << endl;

    vector<Doc> docs;

    auto t_start = chrono::high_resolution_clock::now();
    for (auto i=0; i<1000000; ++i) {
        auto&& this_pk = std::move(Helper::NewPK(12345333));
        Doc mydoc(this_pk, schema);

        mydoc.AddLongFieldValue("age", 20)
             .AddLongFieldValue("likes", 100)
             .AddFloatFieldValue("score", 23.5)
             .AddStringFieldValue("uid", "12443434343")
             .AddFloatVectorFieldValue("vec", {1,2,3,4})
             .Build();

        /* cout << mydoc.Dump() << endl; */
        docs.push_back(std::move(mydoc));
    }
    auto t_end = chrono::high_resolution_clock::now();
    cout << "Create " << docs.size() << " docs ";
    cout << "takes " << chrono::duration<double, std::milli>(t_end-t_start).count() << " ms" << endl;

    return 0;


    map<string, DB*> db_map;
    vector<std::thread> threads;
    make_segments("/home/xupeng/data/ldb", 100, db_map);

    auto start = chrono::high_resolution_clock::now();
    /* std::for_each(db_map.begin(), db_map.end(), [&](const std::pair<string, DB*>& it_pair) { */
    /*     /1* mock_data(it_pair.second, 100000, it_pair.first + ":"); *1/ */
    /*     std::thread t = std::thread(mock_data, it_pair.second, 100000, it_pair.first + ":"); */
    /*     threads.push_back(std::move(t)); */
    /* }); */

    /* for (auto& t : threads) { */
    /*     if (t.joinable()) */
    /*         t.join(); */
    /* } */

    auto end = chrono::high_resolution_clock::now();
    cout << "mock all data takes " << chrono::duration<double, std::milli>(end-start).count() << " ms" << endl;
    threads.clear();

    start = chrono::high_resolution_clock::now();
    std::for_each(db_map.begin(), db_map.end(), [&](const std::pair<string, DB*>& it_pair) {
        auto pre = it_pair.first + ":";
        /* search_seg(it_pair.second, 100000, pre); */
        std::thread t = std::thread(search_seg, it_pair.second, 10000, pre);
        threads.push_back(std::move(t));
    });
    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }
    end = chrono::high_resolution_clock::now();
    cout << "search_seg all data takes " << chrono::duration<double, std::milli>(end-start).count() << " ms" << endl;
    threads.clear();


    destroy_segments(db_map);

    return 0;
    DB *db;
    Options options;
    options.create_if_missing = true;
    Status s = DB::Open(options, "/home/xupeng/data/ldb/db", &db);

    /* mock_data(db, 50000000, "5:"); */
    /* map<string, string> dict; */
    /* load_all(db, dict); */
    search_seg(db, 10000, "");
    /* string key = "name"; */
    /* string value = "xp"; */
    /* auto woptions = WriteOptions(); */
    /* db->Put(woptions, key, value); */
    /* db->Get(ReadOptions(), key, &value); */
    /* cout << key << " : " << value << endl; */

    delete db;

    return 0;
}
