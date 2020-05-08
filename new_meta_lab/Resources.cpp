#include "Resources.h"
#include <sstream>

Collection::Collection(ID_TYPE id, const std::string& name, State status, TS_TYPE created_on) :
    id_(id), name_(name), status_(status), created_on_(created_on) {
}

std::string Collection::ToString() const {
    std::stringstream ss;
    ss << "<ID=" << id_ << ", Name=" << name_ << ", Status=" << status_ << ", TS=" << created_on_ << ">";
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
    if (id_map_.find(collection->GetID()) == id_map_.end()) {
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
