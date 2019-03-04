%{
#include <stdio.h>
%}
%%
[a-zA-Z][a-zA-Z0-9]*    printf("ID ");
[a-zA-Z0-9\/\.\-]+      printf("FILENAME ");
\"  printf("QUOTE ");
\{  printf("OBRACE ");
\}  printf("EBRACE ");
;   printf("SEMICOLON ");
\n  printf("\n");
[ \t]+  /*Ignore*/
%%
