%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
%}

%%
[0-9]+      yylval.number=atoi(yytext); return NUMBER;
heat        return TOKHEAT;
heater        return TOKHEATER;
on|off      yylval.number=!strcmp(yytext, "on"); return STATE;
target      return TOKTARGET;
temperature return TOKTEMPERATURE;
[0-9a-z]+      yylval.string=strdup(yytext); return WORD;
\n          ;
[ \t]+      ;
%%
