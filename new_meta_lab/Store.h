#pragma once
#include "Resources.h"
#include "Schema.h"

#include <stdlib.h>
#include <time.h>
#include <sstream>

class Store {
public:
    static Store& GetInstance() {
        static Store store;
        return store;
    }

    CollectionPtr GetCollection(ID_TYPE id) {
        auto it = id_collections_.find(id);
        if (it == id_collections_.end()) {
            return nullptr;
        }
        auto& c = it->second;
        auto ret = std::make_shared<Collection>(c->GetID(), c->GetName(), c->GetStatus(), c->GetCreatedTime());
        return ret;
    }

    CollectionPtr GetCollection(const std::string& name) {
        auto it = name_collections_.find(name);
        if (it == name_collections_.end()) {
            return nullptr;
        }
        auto& c = it->second;
        auto ret = std::make_shared<Collection>(c->GetID(), c->GetName(), c->GetStatus(), c->GetCreatedTime());
        return ret;
    }

private:
    Store() {
        srand(time(0));
        int random;
        random = rand() % 10 + 10;
        for (auto i=1; i<random; i++) {
            std::stringstream name;
            name << "collection_" << i;
            auto c = std::make_shared<Collection>(i, name.str());
            id_collections_[i] = c;
            name_collections_[name.str()] = c;
        }
    }

    std::map<ID_TYPE, CollectionPtr> id_collections_;
    std::map<std::string, CollectionPtr> name_collections_;
};
