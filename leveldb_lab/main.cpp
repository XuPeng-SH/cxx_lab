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

    LongField age_field;
    LongField likes_field;
    FloatField score_field;
    StringField uid_field;
    FloatVectorField vec_field;

    uid_field.SetMaxLength(20);
    uid_field.SetMinLength(10);

    vec_field.SetMaxLength(4);
    vec_field.SetMinLength(4);

    schema->AddLongField("age", age_field)
           .AddLongField("likes", likes_field)
           .AddFloatField("score", score_field)
           .AddStringField("uid", uid_field)
           .AddFloatVectorField("vec", vec_field)
           .Build();
    cout << schema->Dump() << endl;

    vector<Doc> docs;

    auto t_start = chrono::high_resolution_clock::now();
    for (auto i=0; i<10000; ++i) {
        LongField this_pk;
        this_pk.SetValue(123434343);
        this_pk.Build();
        Doc mydoc(this_pk, schema);
        LongField age;
        age.SetValue(20);
        age.Build();
        LongField likes;
        likes.SetValue(243433);
        likes.Build();
        FloatField score;
        score.SetValue(67.5);
        score.Build();
        StringField uid;
        uid.SetMaxLength(20);
        uid.SetValue("123234343");
        uid.Build();

        FloatVectorField fvec;
        fvec.SetValue({1,2,3,4});
        fvec.Build();

        mydoc.AddLongField("age", age)
             .AddLongField("likes", likes)
             .AddFloatField("score", score)
             .AddStringField("uid", uid)
             .AddFloatVectorField("vec", fvec)
             .Build();

        /* cout << mydoc.Dump() << endl; */
        docs.push_back(std::move(mydoc));
    }
    auto t_end = chrono::high_resolution_clock::now();
    cout << "Create " << docs.size() << " docs ";
    cout << "takes " << chrono::duration<double, std::milli>(t_end-t_start).count() << " ms" << endl;

    return 0;

    LongField lf1;
    LongField lf2;
    schema->AddLongField("age", lf1);
    schema->AddLongField("income", lf2);
    lf1.SetValue(20000);
    lf1.Build();
    lf2.SetValue(20332);
    lf2.Build();

    StringField sf1;
    sf1.SetMaxLength(20);
    sf1.SetMinLength(10);

    schema->AddStringField("uid", sf1);
    schema->AddStringField("age", sf1);
    sf1.SetValue("123234334ssd");
    sf1.Build();

    cout << schema->Dump() << endl;
    schema->Build();
    cout << schema->Dump() << endl;

    auto doc1 = Doc(lf1, schema);
    cout << doc1.GetPK().GetValue() << endl;
    cout << doc1.Build() << endl;

    doc1.AddStringField("uid", sf1);
    doc1.AddStringField("age", sf1);
    cout << doc1.Build() << endl;
    doc1.AddLongField("income", lf2);
    cout << doc1.Build() << endl;

    return 0;
    BooleanField bf;

    bf.SetReadonly(true);
    bf.SetValue(true);
    bf.SetValue(false);

    cout << std::boolalpha;
    cout << bf.GetValue() << endl;

    bf.Build();
    bf.SetValue(false);

    StringField sf;
    sf.SetMaxLength(5);
    sf.SetValue("1234567");
    cout << "sf validate " << sf.Validate() << endl;
    sf.SetValue("567");
    cout << "sf validate " << sf.Validate() << endl;
    cout << "sf validate " << sf.GetValue() << endl;

    IntField inf;
    inf.SetMaxLimit(10);

    inf.SetValue(20);
    cout << "inf Validate " << inf.Validate() << endl;

    inf.SetValue(5);
    cout << "inf Validate " << inf.Validate() << endl;

    FloatVectorField vf;
    vector<float> v = {1,2,3,4, 5, 6, 7, 8, 9};
    vf.SetValue(v);
    cout << "vf Validate " << vf.Validate() << endl;

    vf.SetMaxLength(10);
    vf.SetMinLength(10);
    cout << "vf Validate " << vf.Validate() << endl;
    vf.Build();

    vf.SetMaxLength(10);
    vf.SetMinLength(8);
    cout << "vf Validate " << vf.Validate() << endl;

    auto vf2 = FieldFactory::BuildVectorField<float>(10);
    vf2.SetValue(v);
    cout << "vf2 Validate " << vf2.Validate() << endl;

    vf2.SetValue({1.0,2.0,3,4,5,6,7,8,9,10});
    cout << "vf2 Validate " << vf2.Validate() << endl;

    Doc::PrimaryKeyT pk;
    pk.SetValue(1234);
    pk.Build();



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
