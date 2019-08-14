%{
#include <string>
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.value = t)
%}

%%

[ \t\n]                     ;
[a-zA-Z_][a-zA-Z0-9_]       SAVE_TOKEN; return TIDENTIFIER;
[0-9]+.[0-9]*               SAVE_TOKEN; return TDOUDBLE;
"="                         return TOKEN(TEQAUAL);
"=="                        return TOKEN(TCEQ);
"!="                        return TOKEN(TCNE);
"<"                         return TOKEN(TCLT);
"<="                        return TOKEN(TCLE);
">"                         return TOKEN(TCGT);
">="                        return TOKEN(TCGE);
"("                         return TOKEN(TLPAREN);
")"                         return TOKEN(TRPAREN);
"{"                         return TOKEN(TLBRACE);
"}"                         return TOKEN(TRBRACE);
"."                         return TOKEN(TDOT);
","                         return TOKEN(TCOMMA);
"+"                         return TOKEN(TPLUS);
"-"                         return TOKEN(TMINUS);
"*"                         return TOKEN(TMUL);
"/"                         return TOKEN(TDIV);
.                           printf("Unkown token!\n"); yyterminate();

%%