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
        auto c = id_collections_[id];
        auto ret = std::make_shared<Collection>(c->GetID(), c->GetName(), c->GetStatus(), c->GetCreatedTime());
        return ret;
    }

    CollectionPtr GetCollection(const std::string& name) {
        auto c = name_collections_[name];
        auto ret = std::make_shared<Collection>(c->GetID(), c->GetName(), c->GetStatus(), c->GetCreatedTime());
        return ret;
    }

private:
    Store() {
        srand(time(0));
        int random;
        random = rand() % 10 + 10;
        for (auto i=0; i<random; i++) {
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
