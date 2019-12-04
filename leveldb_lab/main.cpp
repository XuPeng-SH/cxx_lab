#include <string>
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/status.h>

using namespace std;
using namespace leveldb;

int main(int argc, char** argv) {
    DB *db;
    Options options;
    options.create_if_missing = true;
    Status s = DB::Open(options, "/tmp/ldb/db", &db);

    string key = "name";
    string value = "xp";
    auto woptions = WriteOptions();
    db->Put(woptions, key, value);
    db->Get(ReadOptions(), key, &value);
    cout << key << " : " << value << endl;

    return 0;
}
