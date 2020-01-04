#pragma once

#include <rocksdb/db.h>
#include <memory>

namespace db {

const std::shared_ptr<rocksdb::Options>& DefaultOpenOptions();
const std::shared_ptr<rocksdb::WriteOptions>& DefaultDBWriteOptions();

namespace demo {
    void write_batch_demo(std::shared_ptr<rocksdb::DB>);
}

}
