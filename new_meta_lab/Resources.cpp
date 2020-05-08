#include "Resources.h"
#include <sstream>
#include <iostream>

DBBaseResource::DBBaseResource(ID_TYPE id, State status, TS_TYPE created_on) :
    id_(id), status_(status), created_on_(created_on) {
}

std::string DBBaseResource::ToString() const {
    std::stringstream ss;
    ss << "ID=" << id_ << ", Status=" << status_ << ", TS=" << created_on_;
    return ss.str();
}

Collection::Collection(ID_TYPE id, const std::string& name, State status, TS_TYPE created_on) :
    DBBaseResource(id, status, created_on),
    name_(name) {
}

std::string Collection::ToString() const {
    std::stringstream ss;
    ss << "<" << DBBaseResource::ToString() << ", Name=" << name_ << ">";
    return ss.str();
}

CollectionPtr CollectionsHolder::GetCollection(ID_TYPE id) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto cit = id_map_.find(id);
    if (cit == id_map_.end()) {
        return nullptr;
    }
    return cit->second;
}

CollectionPtr CollectionsHolder::GetCollection(const std::string& name) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto cit = name_map_.find(name);
    if (cit == name_map_.end()) {
        return nullptr;
    }
    return cit->second;
}

bool CollectionsHolder::Add(CollectionPtr collection) {
    if (!collection) return false;
    std::unique_lock<std::mutex> lock(mutex_);
    if (id_map_.find(collection->GetID()) != id_map_.end()) {
        return false;
    }

    id_map_[collection->GetID()] = collection;
    name_map_[collection->GetName()] = collection;
    return true;
}

bool CollectionsHolder::Remove(const std::string& name) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = name_map_.find(name);
    if (it == name_map_.end()) {
        return false;
    }

    id_map_.erase(it->second->GetID());
    name_map_.erase(it);
    return true;
}

bool CollectionsHolder::Remove(ID_TYPE id) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = id_map_.find(id);
    if (it == id_map_.end()) {
        return false;
    }

    name_map_.erase(it->second->GetName());
    id_map_.erase(it);
    return true;
}

void CollectionsHolder::Dump(const std::string& tag) {
    std::unique_lock<std::mutex> lock(mutex_);
    std::cout << "CollectionsHolder Dump Start [" << tag <<  "]:" << id_map_.size() << std::endl;
    for (auto& kv : id_map_) {
        std::cout << "\t" << kv.second->ToString() << std::endl;
    }
    std::cout << "CollectionsHolder Dump   End [" << tag <<  "]" << std::endl;
}

CollectionCommit::CollectionCommit(ID_TYPE id, const MappingT& mappings, State status, TS_TYPE created_on) :
    DBBaseResource(id, status, created_on), mappings_(mappings) {
}

std::string CollectionCommit::ToString() const {
    std::stringstream ss;
    ss << "<" << DBBaseResource::ToString() << ", Mappings=" << "[";
    bool first = true;
    std::string prefix;
    for (auto& id : mappings_) {
        if (!first) prefix = ", ";
        else first = false;
        ss << prefix << id;
    }
    ss << "]>";
    return ss.str();
}
