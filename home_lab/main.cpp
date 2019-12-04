#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <gflags/gflags.h>
#include "ast.h"

using namespace std;

DEFINE_string(path, "", "output file path");

int main(int argc, char** argv) {
    /* gflags::ParseCommandLineFlags(&argc, &argv, true); */
    ExprAST* x = new VariableExprAST("x");
    ExprAST* y = new VariableExprAST("y");

    ExprAST* result = new BinaryExprAST('+', x, y);


    delete result;
    delete y;
    delete x;
    return 0;
}
