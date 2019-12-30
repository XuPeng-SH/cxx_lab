#pragma once
#include <rocksdb/db.h>
#include <string>
#include <map>
#include <vector>

using namespace std;
using namespace rocksdb;

using HandleMapT = map<string, vector<ColumnFamilyHandle*>>;
using HandleSelectorT = map<string, ColumnFamilyHandle*>;

void mock_data(DB* db, ColumnFamilyHandle* handle, size_t num, string pre="");
void load_all(DB* db, ColumnFamilyHandle* handle,  map<string, string>& dict);
void search_segments(map<string, DB*>& db_map, HandleSelectorT& selectors, size_t size, const string& prefix, bool async);
void mock_segments_data(map<string, DB*>& db_map, HandleSelectorT& handle_selectors, size_t size, const string& prefix, bool async);
void destroy_segments(map<string, DB*>& db_map, HandleMapT& handles_map);
void make_segments(const string& db_path_prefix, size_t n, map<string, DB*>& db_map,
        HandleMapT& handles_map);
