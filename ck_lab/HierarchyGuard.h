#pragma once

#include <map>
#include <string>
#include <shared_mutex>
#include <memory>
#include <assert.h>
/* #include <iostream> */

template <typename HighIDType, typename LowIDType>
class BaseGuard {
 public:
    struct Entry {
        uint32_t counter_;
        std::unique_ptr<std::mutex> mutex_;
    };

    using Relation = std::map<LowIDType, Entry>;
    using RelationCursorT = typename Relation::iterator;
    using RelationPtr = std::shared_ptr<Relation>;
    using HighMutexPtr = std::shared_ptr<std::shared_mutex>;

    BaseGuard(HighMutexPtr high_mutex, const HighIDType& high_id, RelationPtr rel,
            const LowIDType& low_id,
            std::unique_lock<std::mutex> guard_lock);

    BaseGuard() = delete;
    BaseGuard(const BaseGuard&) = delete;

    ~BaseGuard();

    void
    CleanEntries();

    /// In context of active guard_lock_
    void
    OnNoWaiters();

    /// Unsafe
    uint32_t
    GetEntryCount(const LowIDType& id) const {
        auto it = rel_->find(id);
        if (it == rel_->end()) {
            /* std::cout << "Cannot find" << std::endl; */
            return 0;
        }
        return it->second.counter_;
    }

 private:
    /// High level mutex
    HighMutexPtr high_mutex_;
    RelationPtr rel_;
    RelationCursorT rel_cursor_;
    std::unique_lock<std::mutex> guard_lock_;
    std::unique_lock<std::mutex> entry_lock_;
};

using StrIDGuard = BaseGuard<std::string, std::string>;
using StrIDHierarchyGuard = std::map<std::string, StrIDGuard>;

template <typename HighIDType, typename LowIDType>
BaseGuard<HighIDType, LowIDType>::BaseGuard(typename BaseGuard<HighIDType, LowIDType>::HighMutexPtr high_mutex,
        const HighIDType& high_id, typename BaseGuard<HighIDType, LowIDType>::RelationPtr rel,
        const LowIDType& low_id,
        std::unique_lock<std::mutex> guard_lock)
    :  high_mutex_(high_mutex), rel_(rel), guard_lock_(std::move(guard_lock)) {
    assert(rel_);
    rel_cursor_ = rel_->emplace(low_id, Entry{0, std::make_unique<std::mutex>()}).first;
    ++rel_cursor_->second.counter_;
    /// In above context, guard_lock_ should be active
    /// Unlock guards lock here because the below steps have nothing to do with entries management
    ///        G u a r d
    ///        /   |    \
    /// [entry1, entry2, entry3 ...] ------> Above steps operate on this part
    ///            |
    ///          [...]         ------> Below steps operate on this part. No need of guards lock
    guard_lock_.unlock();
    auto& entry_mutex = *rel_cursor_->second.mutex_;
    entry_lock_ = std::unique_lock<std::mutex>(entry_mutex);
    /// Do some stuff related to specified entry
}

template <typename HighIDType, typename LowIDType>
void
BaseGuard<HighIDType, LowIDType>::OnNoWaiters() {
    /// In context of active guard_lock_
    entry_lock_.unlock();
    rel_->erase(rel_cursor_);
}

template <typename HighIDType, typename LowIDType>
void
BaseGuard<HighIDType, LowIDType>::CleanEntries() {
    guard_lock_.lock();
    --rel_cursor_->second.counter_;
    if (!rel_cursor_->second.counter_) {
        OnNoWaiters();
    }
}

template <typename HighIDType, typename LowIDType>
BaseGuard<HighIDType, LowIDType>::~BaseGuard() {
    CleanEntries();
}
