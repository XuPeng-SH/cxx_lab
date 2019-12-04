#include "lexer.h"
#include "common.h"
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <sstream>

using namespace std;

static char END = '`';

const std::map<int, std::string> TokenStrMap = {
    {tok_undef, "TOK_UNDEF"},
    {tok_id, "TOK_ID"},
    {tok_def, "TOK_DEF"},
    {tok_eof, "TOK_EOF"},
    {tok_num, "TOK_NUM"},
    {tok_extern, "TOK_EXTERN"}
};

std::string IDToken::ToString() const  {
    auto token_str_type = BaseT::ToString();
    stringstream ss;
    ss << token_str_type << ": NameValue=" << NameValue();
    return ss.str();
}

std::string NumToken::ToString() const  {
    auto token_str_type = BaseT::ToString();
    stringstream ss;
    ss << token_str_type << ": NumValue=" << NumValue();
    return ss.str();
}

std::shared_ptr<Token> gettok() {
    static int last_char = ' ';
    string idstr;

    while(isspace(last_char)) {
        last_char = getchar();
        LOGD("get_char: " << (char)last_char);
    }

    if (isalpha(last_char)) {
        idstr = last_char;
        while(isalnum(last_char=getchar())) {
            idstr += last_char;
        }

        LOGD("[TOK_ID] idstr: " << idstr);
        if(idstr == "def") return std::make_shared<DefToken>();
        if(idstr == "extern") return std::make_shared<ExternToken>();
        return std::make_shared<IDToken>(idstr);
    }

    double num_val;
    if (isdigit(last_char) || last_char == '.') {
        string num_str;
        do {
            num_str += last_char;
            last_char = getchar();
        } while (isdigit(last_char) || last_char == '.');

        num_val = strtod(num_str.c_str(), 0);
        LOGD("[TOK_NUM] num_val: " << num_val);
        return std::make_shared<NumToken>(num_val);
    }

    if (last_char == '#') {
        string commented;
        do {
            last_char = getchar();
            commented += last_char;
        }
        while(last_char != EOF && last_char != '\n' && last_char != '\r' && last_char != END);
        if (last_char != EOF && last_char != END) {
            LOGD("commented: " << commented);
            return gettok();
        }
    }

    if (last_char == EOF || last_char == END) {
        LOGD("[TOK_EOF]");
        return std::make_shared<EOFToken>();
    }


    return make_shared<Token>();

    /* int this_char = last_char; */
    /* LOGD("this_char: " << (char)this_char); */
    /* last_char = getchar(); */
    /* LOGD("last_char: " << (char)last_char); */
    /* return this_char; */
}
