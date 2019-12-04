#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include <memory>

enum TokenType {
    tok_undef = 0,
    tok_eof = -1,
    tok_def = -2,
    tok_extern = -3,
    tok_id = -4,
    tok_num = -5
};

extern const std::map<int, std::string> TokenStrMap;

class Token {
protected:
    const TokenType type_;
    Token(TokenType type) : type_(type) {}

public:
    Token() : type_(tok_undef) {}
    virtual ~Token() {}
    virtual double NumValue() const {
        throw std::runtime_error(std::string(__func__) + " should not be used");
    }

    virtual const std::string& NameValue() const {
        throw std::runtime_error(std::string(__func__) + " should not be used");
    }

    virtual std::string ToString() const {
        return TokenStrMap.at(type_);
    }

    bool Valid() const {
        return type_ != tok_undef;
    }

    bool End() const {
        return type_ == tok_eof;
    }
};

class DefToken : public Token {
public:
    DefToken() : Token(tok_def) {}
};

class EOFToken : public Token {
public:
    EOFToken() : Token(tok_eof) {}
};

class ExternToken : public Token {
public:
    ExternToken() : Token(tok_extern) {}
};

class IDToken : public Token {
    const std::string id_name_;
public:
    using BaseT = Token;
    IDToken(const std::string& id_name) : Token(tok_id), id_name_(id_name) {}
    const std::string& NameValue() const override {
        return id_name_;
    }

    std::string ToString() const override;
};

class NumToken : public Token {
    const double num_;
public:
    using BaseT = Token;
    NumToken(double num) : Token(tok_num), num_(num) {}
    double NumValue() const override {
        return num_;
    }
    std::string ToString() const override;
};

std::shared_ptr<Token> gettok();
