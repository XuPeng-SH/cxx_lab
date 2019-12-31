#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>

typedef enum {
    INVALID = -1,
    INT = 0,
    LONG,

    BOOL = 100,

    FLOAT = 200,
    DOUBLE,

    STRING = 300,

    VECTOR = 400

} TypeENUM;


struct DoubleField {
    static const TypeENUM Type = TypeENUM::DOUBLE;
    DoubleField(double val);
    double val;
};

struct IntField {
    static const TypeENUM Type = TypeENUM::INT;
    IntField(int val);
    int val;
};

struct StringField {
    static const TypeENUM Type = TypeENUM::STRING;
    StringField(const std::string& val);
    std::string val;
};

/* struct BooleanField : public Field { */

/* }; */

/* struct TableSchema { */
/*     std::string name; */
/*     std::map<std::string, std::string> fields; */
/*     std::map<std::string, > fields_params */
/* }; */

/* class DBImpl; */
/* class MyDB { */
/* public: */
/*     CreateTable(const std::string& table_name); */

/* private: */
/*     std::shared_ptr<DBImpl> impl_; */
/* }; */
