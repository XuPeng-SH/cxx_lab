%{
#include <stdio.h>
#include <string.h>

#define YYSTYPE char *

void yyerror(const char* str) {
    fprintf(stderr, "error: %s\n", str);
}

int yywrap() {
    return 1;
}

int main() {
    yyparse();
    return 0;
}
%}

%token ZONETOK FILENAME FILETOK QUOTE OBRACE EBRACE SEMICOLON WORD

%%

commands:
        |commands command SEMICOLON
        ;

command:
       zone_set;
       ;

quotedname:
       QUOTE FILENAME QUOTE
       {
          $$ = $2;
       }
       ;

block:
     OBRACE zonestatements EBRACE SEMICOLON
    ;

statements:
       | statements statement
       ;

zonestatements:
       |
       zonestatements zonestatement SEMICOLON
        ;

zonestatement:
       statements
        |
       FILETOK quotedname
       {
         printf("A zonefile name '%s' was encountered\n", $2);
       }
       ;

zonecontent:
        OBRACE zonestatements EBRACE
        ;

zone_set:
       ZONETOK quotedname zonecontent
       {
        printf("Complete zone for '%s' found\n", $2);
       }
       ;

statement: WORD | block | quotedname
         ;

%%
