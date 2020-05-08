#pragma once

#include "Helper.h"
#include <string>
#include <map>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>

enum State {
    PENDING = 0,
    ACTIVE = 1,
    DEACTIVE = 2
};

using ID_TYPE = int64_t;
using TS_TYPE = int64_t;

/* class Resource { */

/* }; */

class Collection {
public:

    Collection(ID_TYPE id, const std::string& name, State status = PENDING,
            TS_TYPE created_on = GetMicroSecTimeStamp());

    bool IsActive() const {return status_ == ACTIVE;}
    bool IsDeactive() const {return status_ == DEACTIVE;}

    ID_TYPE GetID() const {return id_;}
    State GetStatus() const {return status_;}
    TS_TYPE GetCreatedTime() const {return created_on_;}
    const std::string& GetName() const {return name_;}

    std::string ToString() const;

private:
    ID_TYPE id_;
    std::string name_;
    State status_;
    TS_TYPE created_on_;
};

using CollectionPtr = std::shared_ptr<Collection>;

class CollectionsHolder {
public:
    using NameMapT = std::map<std::string, CollectionPtr>;
    using IdMapT = std::map<ID_TYPE, CollectionPtr>;

    CollectionPtr GetCollection(ID_TYPE id);
    CollectionPtr GetCollection(const std::string& name);

    bool Add(CollectionPtr collection);
    bool Remove(const std::string& name);
    bool Remove(ID_TYPE id);

    void Dump(const std::string& tag = "");

private:
    std::mutex mutex_;
    NameMapT name_map_;
    IdMapT id_map_;
};

using CollectionsHolderPtr = std::shared_ptr<CollectionsHolder>;
