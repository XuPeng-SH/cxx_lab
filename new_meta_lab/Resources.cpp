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
