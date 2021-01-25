#pragma once

#include <string>
#include <sstream>
#include <memory.h>

struct UserSchema {
    constexpr static const uint16_t NameSize = 32;
    constexpr static const uint16_t EMailSize = 255;
    uint32_t id;
    char username[NameSize];
    char email[EMailSize];

    void
    SetUserName(const char* un) {
        memset(username, 0, NameSize);
        if (!un) return;
        strncpy(username, un, sizeof(username));
    }
    void
    SetEmail(const char* e) {
        memset(email, 0, EMailSize);
        if (!e) return;
        strncpy(email, e, sizeof(email));
    }

    void
    SerializeTo(void* destination) const {
        memcpy(destination, this, sizeof(UserSchema));
    }
    void
    DeserializeFrom(void* source) {
        memcpy((void*)this, source, sizeof(UserSchema));
    }

    std::string
    ToString() const {
        std::stringstream ss;
        ss << "(" << id << ", " << username << ", " << email << ")";
        return ss.str();
    }
};
