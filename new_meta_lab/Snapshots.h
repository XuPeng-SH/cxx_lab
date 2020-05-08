#pragma once
#include "Resources.h"
#include <map>
#include <memory>
#include <deque>
#include <string>
#include <vector>
#include <assert.h>
#include <iostream>

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

class Snapshot {
public:
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

    CollectionCommit::Ptr collection_commit_;
    Collection::Ptr collection_;

};

Snapshot::Snapshot(ID_TYPE id) {
    collection_commit_ = CollectionCommitsHolder::GetInstance().GetResource(id);
    assert(collection_commit_);
    collection_ = CollectionsHolder::GetInstance().GetResource(collection_commit_->GetCollectionId());
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
}

/* class CollectionSnapshots { */
/* public: */

/* private: */
/*     std::map<ID_TYPE, Snapshot> snapshots_; */
/* }; */

/* class Snapshots { */


/* private: */
/* }; */
