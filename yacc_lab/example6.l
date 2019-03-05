%{
#include <stdio.h>
%}

%s expect

%%

expect-floats   BEGIN(expect);

<expect>\n      {BEGIN(INITIAL);}

<expect>[0-9]+"."[0-9]+ {
                            printf("found a float, = %f\n", atof(yytext));
                        }

[0-9]+ {
         printf("found an integer %d\n", atoi(yytext));
       }

"." printf("found a dot\n");

%%
