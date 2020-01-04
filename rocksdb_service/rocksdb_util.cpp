#include "rocksdb_util.h"
#include <iostream>
#include <string>

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
        options->disableWAL = true;
        options->sync = false;
    }
    return options;
}

}
