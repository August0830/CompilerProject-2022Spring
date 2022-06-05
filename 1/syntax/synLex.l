%{
    #include "syntax.tab.h"
%}
digit [0-9]
ADD "+"
SUB "-"
MUL "*"
DIV "/"
%%
{digit}* {
    yylval = atoi(yytext);
    return INT;
}
{ADD} {return ADD;}
{SUB} {return SUB;}
{MUL} {return MUL;}
{DIV} {return DIV;}
\n {yylineno++;}
. {printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno,yytext);}
%%