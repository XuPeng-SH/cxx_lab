##Lex and YACC examples [exmaples](./yacc_lab)
###Compile Guide
1. Compile lexer
```lex examplex.l```
2. Compile with yacc
```yacc -d examplex.y```
3. Compile together
```cc lex.yy.c y.tab.c -o examplex```
4. Run
```./examplex```
