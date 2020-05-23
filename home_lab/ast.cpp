#include "ast.h"

ExprAST* Error(const char* str) {
    fprintf(stderr, "Error: %s\n", str); return 0;
}

PrototypeExprAST* ErrorP(const char* str) {
    Error(str); return 0;
}

FunctionExprAST* ErrorF(const char* str) {
    Error(str); return 0;
}

/* static ExprAST* ParseNumberExpr() { */
/*     ExprAST* result = new NumberExprAST() */
/* } */

static ExprAST* ParseParentExpr() {

}
