#pragma once

#include "Helper.h"
#include <string>
#include <map>
#include <vector>
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
using MappingT = std::vector<ID_TYPE>;

class DBBaseResource {
public:
    DBBaseResource(ID_TYPE id, State status, TS_TYPE created_on);

    bool IsActive() const {return status_ == ACTIVE;}
    bool IsDeactive() const {return status_ == DEACTIVE;}

    ID_TYPE GetID() const {return id_;}
    State GetStatus() const {return status_;}
    TS_TYPE GetCreatedTime() const {return created_on_;}

    virtual std::string ToString() const;

    virtual ~DBBaseResource() {}

protected:
    ID_TYPE id_;
    State status_;
    TS_TYPE created_on_;
};


class Collection : public DBBaseResource {
public:

    Collection(ID_TYPE id, const std::string& name, State status = PENDING,
            TS_TYPE created_on = GetMicroSecTimeStamp());

    const std::string& GetName() const {return name_;}

    std::string ToString() const override;

private:
    std::string name_;
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

class CollectionCommit : public DBBaseResource {
public:
    CollectionCommit(ID_TYPE id, const MappingT& mappings = {}, State status = PENDING,
            TS_TYPE created_on = GetMicroSecTimeStamp());

    const MappingT& GetMappings() const { return mappings_; }

    std::string ToString() const override;

private:
    MappingT mappings_;
};
