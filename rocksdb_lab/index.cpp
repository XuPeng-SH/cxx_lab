#include "index.h"

IndexPtr IndexHub::GetIndex(uint64_t tid, uint64_t sid) {
    auto key = IndexKeyT(tid, sid);
    auto it = indice_.find(key);
    if (it == indice_.end()) {
        return nullptr;
    }
    return it->second;
}
