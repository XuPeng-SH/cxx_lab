#pragma once

#include <rocksdb/db.h>
#include <memory>

namespace db {

const std::shared_ptr<rocksdb::Options>& DefaultOpenOptions();
const std::shared_ptr<rocksdb::WriteOptions>& DefaultDBWriteOptions();

}
