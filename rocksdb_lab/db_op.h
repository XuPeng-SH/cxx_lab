#pragma once
#include <rocksdb/db.h>
#include <string>
#include <map>

using namespace std;
using namespace rocksdb;

void mock_data(DB* db, size_t num, string pre="");
void load_all(DB* db, map<string, string>& dict);
/* void search_seg(DB* db, size_t n, string pre=""); */
void make_segments(const string& db_path_prefix, size_t n, map<string, DB*>& db_map);
void search_segments(map<string, DB*>& db_map, size_t size, const string& prefix, bool async);
void mock_segments_data(map<string, DB*>& db_map, size_t size, const string& prefix, bool async);
void destroy_segments(map<string, DB*>& db_map);
