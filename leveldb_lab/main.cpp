#include <string>
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/status.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <chrono>
#include <map>

using namespace std;
using namespace leveldb;

void mock_data(DB* db, size_t num, string pre="") {

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

void search(DB* db, size_t n, string pre="", bool debug=false) {
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

int main(int argc, char** argv) {
    DB *db;
    Options options;
    options.create_if_missing = true;
    Status s = DB::Open(options, "/home/xupeng/data/ldb/db", &db);

    /* mock_data(db, 50000000, "5:"); */
    /* map<string, string> dict; */
    /* load_all(db, dict); */
    search(db, 10000, "", false);
    /* string key = "name"; */
    /* string value = "xp"; */
    /* auto woptions = WriteOptions(); */
    /* db->Put(woptions, key, value); */
    /* db->Get(ReadOptions(), key, &value); */
    /* cout << key << " : " << value << endl; */

    delete db;

    return 0;
}
