%{
    #include "node.h"
    #include "syntax.tab.h"
   
    extern unsigned int lexError;
    extern int yylineno;
    int yycolumn = 1;

    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno;\
    yylloc.first_column = yycolumn;\
    yylloc.last_column = yycolumn + yyleng -1;\
    yycolumn += yyleng;
%}

%option yylineno

SPACE [ \t]

STRUCT struct 
RETURN return
IF if
ELSE else
WHILE while
TYPE int|float
RELOP >|<|>=|<=|==|!=

PLUS "+"
SUB "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
SEMI ";"
COMMA ","
ASSIGNOP "="

LP "("
RP ")"
LC "{"
RC "}"
LB "["
RB "]"

ID [_a-zA-Z][a-zA-Z_0-9]*
OCT 0[0-7]*
NONOCT 0[89]*
HEX 0[xX][0-9A-Fa-f]*
NONHEX 0[xX][G-Zg-z]*
INT 0|(1[0-9]{9}|20[0-9]{8}|213[0-9]{7}|2146[0-9]{6}|21473[0-9]{5}|214747[0-9]{4}|2147482[0-9]{3}|21474835[0-9]{2}|214748363[0-9]|214748364[0-7]|[1-9][0-9]{0,8})|-2147483648
FLOAT ([0-9]+)"."[0-9]+
%%
{SPACE}+ {;}
\n {;yycolumn = 1;}
{IF}        {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"IF",val);return IF;}
{ELSE}      {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"ELSE",val);return ELSE;}
{WHILE}     {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"WHILE",val);return WHILE;}
{STRUCT}    {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"STRUCT",val);return STRUCT;}
{RETURN}    {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"RETURN",val);return RETURN;}
{TYPE}      {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_TYPE,yylineno,"TYPE",val);return TYPE;}

{RELOP}     {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"RELOP",val);return RELOP;}
{PLUS}      {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"PLUS",val);return PLUS;}
{SUB}       {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"SUB",val);return SUB;}
{STAR}      {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"STAR",val);return STAR;}
{DIV}       {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"DIV",val);return DIV;}
{AND}       {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"AND",val);return AND;}
{OR}        {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"OR",val);return OR;}
{NOT}       {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"NOT",val);return NOT;}

{DOT}       {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"DOT",val);return DOT;}
{SEMI}      {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"SEMI",val);return SEMI;}
{COMMA}     {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"COMMA",val);return COMMA;}
{ASSIGNOP}  {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"ASSIGNOP",val);return ASSIGNOP;}

{LP} {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"LP",val);return LP;}
{RP} {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"RP",val);return RP;}
{LB} {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"LB",val);return LB;}
{RB} {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"RB",val);return RB;}
{LC} {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"LC",val);return LC;}
{RC} {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_OTHER,yylineno,"RC",val);return RC;}

{ID} {ValType val;val.val = yytext;yylval.node = newTerminatorNode(TOKEN_ID,yylineno,"ID",val);return ID;}
{OCT} {
    int demic = 0;
    int times = 1;
    for(int i=strlen(yytext)-1;i>0;--i){
        char str[1] = {};
        str[0] = yytext[i];
        demic += times * atoi(str);
        times *= 8;
    }
    ValType val;
    val.iVal = demic;
    yylval.node = newTerminatorNode(TOKEN_INT,yylineno,"INT",val);
    return INT;
    }
{HEX} {
    int demic = 0;
    int times = 1;
    for(int i=strlen(yytext)-1;i>1;--i){
        char ch[1] = {};
        ch[0] = yytext[i];
        int val = 0;
        switch(ch[0]){
            case 'A': 
            case 'a': val = 10;break;
            case 'B':
            case 'b': val = 11;break;
            case 'C':
            case 'c': val = 12;break;
            case 'D':
            case 'd': val = 13;break;
            case 'E':
            case 'e': val = 14;break;
            case 'F':
            case 'f': val = 15;break;
            default: val = atoi(ch);
        }
        demic += val * times;
        times *= 16;
    }
    ValType val;
    val.iVal = demic;
    yylval.node = newTerminatorNode(TOKEN_INT,yylineno,"INT",val);
    return INT;
    }
{INT} {
    int demic = 0;
    if(yytext[0]=='-'){
        char* str = (char*)malloc(sizeof(char)*(strlen(yytext)-1));
        strcpy(str,yytext+1);
        demic = -1 * atoi(str);
    }else{
        demic = atoi(yytext);
    }
    ValType val;
    val.iVal = demic;
    yylval.node = newTerminatorNode(TOKEN_INT,yylineno,"INT",val);
    return INT;
    }
{FLOAT} {
    ValType val;
    val.val = yytext;
    yylval.node = newTerminatorNode(TOKEN_FLOAT,yylineno,"FLOAT",val);
    return FLOAT;
    }
{NONOCT} {lexError=1;printf("Error type A at Line %d: Wrong OCT Number \'%s\'\n",yylineno,yytext);return NONOCT;}
{NONHEX} {lexError=1;printf("Error type A at Line %d:Wrong HEX Number \'%s\'\n",yylineno,yytext);return NONHEX;}
. {lexError=1;printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno,yytext);}
%%