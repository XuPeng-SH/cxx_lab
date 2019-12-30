#include "db_op.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

void mock_data(DB* db, size_t num, string pre) {
  cout << __func__ << " num=" << num << " pre=" << pre << endl;

  auto write_options = WriteOptions();

  string key_prefix = pre + "mock_prefix_user_key:";
  string val_prefix = "mock_prefix_user_value:";
  auto start = chrono::high_resolution_clock::now();
  for (auto i=0; i<num; ++i) {
      auto strnum = to_string(i);
      db->Put(write_options, key_prefix + strnum, val_prefix + strnum);
      if (i % 5000000 == 0 && i > 0) {
          cout << "handling the " << i << " th" << endl;
      }
  }
  auto end = chrono::high_resolution_clock::now();
  cout << __func__ << " takes " << chrono::duration<double, std::milli>(end-start).count() << endl;
}

void load_all(DB* db, map<string, string>& dict) {
    size_t count = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
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

void search_seg(DB* db, size_t n, string pre) {
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

void mock_segments_data(map<string, DB*>& db_map, size_t size, const string& prefix, bool async) {
    vector<std::thread> threads;
    auto start = chrono::high_resolution_clock::now();
    std::for_each(db_map.begin(), db_map.end(), [&](const std::pair<string, DB*>& it_pair) {
        auto prefix_key = it_pair.first + (prefix != "" ? ":" + prefix : "");
        if (!async) {
            mock_data(it_pair.second, size, prefix_key);
        } else {
            std::thread t = std::thread(mock_data, it_pair.second, size, prefix_key);
            threads.push_back(std::move(t));
        }
    });

    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }

    auto end = chrono::high_resolution_clock::now();
    cout << __func__  << " takes " << chrono::duration<double, std::milli>(end-start).count() << " ms" << endl;
    threads.clear();
}

void search_segments(map<string, DB*>& db_map, size_t size, const string& prefix, bool async) {
    vector<std::thread> threads;
    auto start = chrono::high_resolution_clock::now();
    std::for_each(db_map.begin(), db_map.end(), [&](const std::pair<string, DB*>& it_pair) {
        auto pre = it_pair.first + (prefix != "" ? ":" + prefix : "");
        if (!async) {
            search_seg(it_pair.second, size, pre);
        } else {
            std::thread t = std::thread(search_seg, it_pair.second, size, pre);
            threads.push_back(std::move(t));
        }
    });
    for (auto& t : threads) {
        if (t.joinable())
            t.join();
    }
    auto end = chrono::high_resolution_clock::now();
    cout << __func__ << " takes " << chrono::duration<double, std::milli>(end-start).count() << " ms" << endl;
    threads.clear();


}
