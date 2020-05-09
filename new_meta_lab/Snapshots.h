#pragma once
#include "Resources.h"
#include <map>
#include <memory>
#include <deque>
#include <string>
#include <vector>
#include <assert.h>
#include <iostream>
#include <limits>
#include <cstddef>
#include <mutex>
#include <thread>


/* struct Node { */
/*     using Ptr = std::shared_ptr<Node>; */
/*     bool IsHead() const { return prev_ == nullptr; } */
/*     bool IsTail() const { return next_ == nullptr; } */

/* privtae: */
/*     friend class LinkList; */
/*     Ptr prev_ = nullptr; */
/*     Ptr next_ = nullptr; */
/* }; */

/* class LinkList { */
/* public: */
/*     void Append(Node::Ptr node) { */
/*         if (!head_) { */
/*             head_ = node; */
/*             tail_ = node; */
/*             return; */
/*         } */

/*         node->prev_ = tail_; */
/*         tail_ = node; */
/*         node->next_ = nullptr; */
/*     } */

/* private: */
/*     Node::Ptr head_ = nullptr; */
/*     Node::Ptr tail_ = nullptr; */
/* }; */


using Strings = std::vector<std::string>;
using Collections = std::vector<Collection>;

using CollectionScopedPtr = ScopedResource<Collection>::Ptr;
using CollectionCommitScopedPtr = ScopedResource<CollectionCommit>::Ptr;

class Snapshot {
public:
    using Ptr = std::shared_ptr<Snapshot>;
    Snapshot(ID_TYPE id);
    // TODO
    /* status DescribeCollection(CollectionSchema& schema) */

private:
    /* PartitionCommits */
    /* Partitions */
    /* Schema::Ptr */
    /* FieldCommits */
    /* Fields */
    /* FieldElements */
    /* SegmentCommits */
    /* Segments */
    /* SegmentFiles */

    /* CollectionCommit::Ptr collection_commit_; */
    CollectionScopedPtr collection_;
    CollectionCommitScopedPtr collection_commit_;
    /* Collection::Ptr collection_; */

};

Snapshot::Snapshot(ID_TYPE id) {
    collection_commit_ = CollectionCommitsHolder::GetInstance().GetResource(id);
    assert(collection_commit_);
    collection_ = CollectionsHolder::GetInstance().GetResource(collection_commit_->Get()->GetCollectionId());
    /* auto& mappings =  collection_commit_->GetMappings(); */
    /* auto& partition_commits_holder = PartitionCommitsHolder::GetInstance(); */
    /* auto& partitions_holder = PartitionsHolder::GetInstance(); */
    /* for (auto& id : mappings) { */
    /*     partition_commit = partition_commits_holder.GetResource(id); */
    /*     partition = partitions_holder.GetResource(partition_commit->GetPartitionID()); */
    /*     partition_commits_[partition_commit->GetPartitionID()] = partition_commit; */
    /*     partitions_[partition_commit->GetPartitionID()] = partition; */
    /*     auto& s_c_mappings = partition_commit->GetMappings(); */
    /*     for (auto& s_c_id : s_c_mappings) { */
    /*         segment_commit = segment_commits_holder.GetResource(s_c_id); */
    /*         segment = segments_holder.GetResource(segment_commit->GetSegmentID()); */
    /*         segment_commits_[segment_commit->GetSegmentID()] = segment_commit; */
    /*         segments_[segment_commit->GetGetSegmentID()] = segment; */
    /*         auto& s_f_mappings = segment_commit->GetMappings(); */
    /*         for (auto& s_f_id : s_f_mappings) { */
    /*             segment_file = segment_files_holder.GetResource(s_f_id); */
    /*             segment_files_[segment_commit->GetSegmentID()][s_f_id] = segment_file; */
    /*         } */
    /*     } */
    /* } */
    /* schema_commit = SchemaCommitsHolder::GetInstance().GetResource(collection_commit_->GetSchemaCommitId()); */
    /* auto& f_c_mappings =  schema_commit->GetMappings(); */
    /* for (auto& f_c_id : f_c_mappings) { */
    /*     field_commit = field_commits_holder.GetResource(f_c_id); */
    /*     field = fields_holder.GetResource(field_commit->GetFieldID()); */
    /*     field_commits_[field_commit->GetFieldID()] = field_commit; */
    /*     fields_[fields_commit->GetFieldID()] = field; */
    /*     auto& f_e_mappings = field_commit->GetMappings(); */
    /*     for (auto& f_e_id : f_e_mappings) { */
    /*         field_element = field_elements_holder.GetResource(f_e_id); */
    /*         field_elements_[field_commit->GetFieldID()][f_e_id] = field_element; */
    /*     } */
    /* } */
};

class SnapshotsHolder {
public:
    /* static SnapshotsHolder& GetInstance() { */
    /*     static SnapshotsHolder holder; */
    /*     return holder; */
    /* } */
    SnapshotsHolder(size_t num_versions = 1) : num_versions_(num_versions) {}
    bool Add(ID_TYPE id) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (active_.size() > 0 && id < max_id_) {
                return false;
            }
        }
        auto ss = std::make_shared<Snapshot>(id);

        std::unique_lock<std::mutex> lock(mutex_);
        auto it = active_.find(id);
        if (it != active_.end()) {
            return false;
        }

        if (min_id_ > id) {
            min_id_ = id;
        }

        if (max_id_ < id) {
            max_id_ = id;
        }

        active_[id] = ss;
        if (active_.size() <= num_versions_)
            return true;

        auto oldest_it = active_.find(min_id_);
        auto oldest_ss = oldest_it->second;
        active_.erase(oldest_it);
        ReadyForRelease(oldest_ss); // TODO: Use different mutex
        min_id_ = active_.begin()->first;
        return true;
    }


private:
    void ReadyForRelease(Snapshot::Ptr ss) {
        to_release_.push_back(ss);
    }

    /* SnapshotsHolder()  = default; */
    /* ~SnapshotsHolder() = default; */
    std::mutex mutex_;
    ID_TYPE min_id_ = std::numeric_limits<ID_TYPE>::max();
    ID_TYPE max_id_ = std::numeric_limits<ID_TYPE>::min();
    std::map<ID_TYPE, Snapshot::Ptr> active_;
    std::vector<Snapshot::Ptr> to_release_;
    size_t num_versions_ = 1;
};

/* class CollectionSnapshots { */
/* public: */

/* private: */
/*     std::map<ID_TYPE, Snapshot> snapshots_; */
/* }; */

/* class Snapshots { */


/* private: */
/* }; */
