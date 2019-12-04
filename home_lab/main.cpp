#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <gflags/gflags.h>
#include <iostream>
#include <vector>
#include "ast.h"
#include "lexer.h"
#include "common.h"

using namespace std;

DEFINE_string(path, "", "output file path");

int main(int argc, char** argv) {
    /* cout << "XXXXXXXXXXXXXXXX" << endl; */
    /* gflags::ParseCommandLineFlags(&argc, &argv, true); */
    ExprAST* x = new VariableExprAST("x");
    ExprAST* y = new VariableExprAST("y");

    ExprAST* result = new BinaryExprAST('+', x, y);
    std::shared_ptr<Token> curr_tok;

    vector<std::shared_ptr<Token>> tokens;

    while((curr_tok = gettok()) && curr_tok->Valid() && !(curr_tok->End())) {
        tokens.push_back(curr_tok);
    }

    for(auto tok : tokens) {
        LOGD(tok->ToString());
    }

    delete result;
    delete y;
    delete x;
    return 0;
}
