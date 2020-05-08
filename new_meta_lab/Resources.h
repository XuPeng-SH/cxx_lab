#pragma once

#include "Helper.h"
#include <string>

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
