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
    /* Partitions partitions_; */
    CollectionCommit::Ptr collection_commit_;
    Collection::Ptr collection_;

};

Snapshot::Snapshot(ID_TYPE id) {
    collection_commit_ = CollectionCommitsHolder::GetInstance().GetResource(id);
    assert(collection_commit_);
    collection_ = CollectionsHolder::GetInstance().GetResource(collection_commit_->GetCollectionId());
    auto& mappings =  collection_commit_->GetMappings();
    for (auto& id : mappings) {
        std::cout << id << std::endl;
    }
    /* if (!collection_commit_) { */
    /*     std::cout << "Snapshot is Empty" << std::endl; */
    /* } else { */
    /*     std::cout << "Has !!" << std::endl; */
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
