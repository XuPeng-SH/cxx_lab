#include "rocksdb_util.h"
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

namespace db {

const std::shared_ptr<rocksdb::Options>& DefaultOpenOptions() {
    static std::shared_ptr<rocksdb::Options> options;
    if (!options) {
        options = std::make_shared<rocksdb::Options>();
        options->create_if_missing = true;
    }
    return options;
}

const std::shared_ptr<rocksdb::WriteOptions>& DefaultDBWriteOptions() {
    static std::shared_ptr<rocksdb::WriteOptions> options;
    if (!options) {
        options = std::make_shared<rocksdb::WriteOptions>();
        options->disableWAL = false;
        options->sync = false;
    }
    return options;
}

namespace demo {

void read_all(std::shared_ptr<rocksdb::DB> db) {
    size_t count = 0;
    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    std::map<std::string, std::string> dict;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        count++;
        std::cout << "(" << it->key().ToString() << "," << it->value().ToString() << ")" << std::endl;
    }
    std::cout << "Found " << count << " KVs" << std::endl;
}

void write_batch_demo(std::shared_ptr<rocksdb::DB> db) {
    std::vector<float> vec(512);
    for (auto i=0; i<512; ++i) {
        vec[i] = 0.0001 * i;
    }

    rocksdb::WriteBatch write_batch;
    size_t cap = 8000;
    std::vector<std::string> keys;
    std::vector<std::string> values;
    for (auto i=0; i<cap; ++i) {
        std::string aname_key = "A:";
        std::string aname_val;
        aname_key.append((char*)&i, (size_t)(sizeof(int)));
        aname_val.append((char*)&i, (size_t)(sizeof(int)));

        keys.push_back(std::move(aname_key));
        values.push_back(std::move(aname_val));

        std::string bname_key = "B:";
        std::string bname_val;
        bname_key.append((char*)&i, (size_t)(sizeof(int)));
        bname_val.append((char*)&i, (size_t)(sizeof(int)));

        keys.push_back(std::move(bname_key));
        values.push_back(std::move(bname_val));

        std::string cname_key = "C:";
        std::string cname_val;
        cname_key.append((char*)&i, (size_t)(sizeof(int)));
        cname_val.append((char*)&i, (size_t)(sizeof(int)));
        write_batch.Put(cname_key, cname_val);

        keys.push_back(std::move(cname_key));
        values.push_back(std::move(cname_val));
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (auto i=0; i<keys.size(); ++i) {
        write_batch.Put(keys[i], values[i]);
    }

    std::cout << write_batch.Count() << std::endl;
    auto s = db->Write(*DefaultDBWriteOptions(), &write_batch);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << __func__ << " takes " << std::chrono::duration<double, std::milli>(end-start).count() << " ms" << std::endl;

    if (!s.ok()) {
        std::cout << s.ToString() << std::endl;
    }
    /* read_all(db); */
}

}

}
