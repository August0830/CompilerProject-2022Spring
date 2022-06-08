%locations

%{
    #include <stdio.h>
    #include "lex.yy.c"

    ptrNode root;
    extern int synError;
%}

%union {
    ptrNode node;
}

%token <node> INT 
%token <node> HEX 
%token <node> NONHEX NONOCT
%token <node> OCT
%token <node> FLOAT
%token <node> ID
%token <node> TYPE
%token <node> COMMA
%token <node> DOT
%token <node> SEMI
%token <node> RELOP
%token <node> ASSIGNOP 
%token <node> PLUS SUB STAR DIV
%token <node> AND OR NOT 
%token <node> LP RP LC RC LB RB 
%token <node> IF
%token <node> ELSE
%token <node> WHILE
%token <node> STRUCT 
%token <node> RETURN 

%type <node> Program ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt
%type <node> DefList Def Dec DecList
%type <node> Exp Args

%right ASSIGNOP
%left OR
%left AND 
%left RELOP
%left PLUS SUB
%left STAR DIV
%right NOT
%left DOT
%left LB RB
%left LP RP  

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/*High Level Definitions*/
Program :     ExtDefList                {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Program",1,$1);root = $$;}
  ; 
ExtDefList : /*empty*/                  {$$ = NULL;}
  |           ExtDef ExtDefList         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"ExtDefList",2,$1,$2);}
  ;
ExtDef :      Specifier ExtDecList SEMI {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"ExtDef",3,$1,$2,$3);}
  |           Specifier SEMI            {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"ExtDef",2,$1,$2);}
  |           Specifier FunDec CompSt  {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"ExtDef",3,$1,$2,$3);}
  |           error SEMI                {synError = 1;}
  ;
ExtDecList :  VarDec                    {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"ExtDecList",1,$1);}
  |           VarDec COMMA ExtDecList   {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"ExtDecList",3,$1,$2,$3);}
  ;

/*Specifiers*/
Specifier : TYPE                        {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Specifier",1,$1);}
  | StructSpecifier                     {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Specifier",1,$1);}
  ;
StructSpecifier : STRUCT OptTag LC DefList RC {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"StructSpecifier",5,$1,$2,$3,$4,$5);}
  | STRUCT Tag                          {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"StructSpecifier",2,$1,$2);}
  ;
OptTag : /*empty*/                      {$$ = NULL;}
  | ID                                  {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"OptTag",1,$1);}
  ;
Tag : ID                               {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Tag",1,$1);}
  ;
/*Declarators*/
VarDec : ID                             {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"VarDec",1,$1);}
  | VarDec LB INT RB                    {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"VarDec",4,$1,$2,$3,$4);}
  | error RB                            {synError = 1;}
  ;
FunDec : ID LP VarList RP              {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"FunDec",4,$1,$2,$3,$4);}      
  | ID LP RP                            {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"FunDec",3,$1,$2,$3);}
  | error RP                            {synError = 1;}
  ;
VarList : ParamDec COMMA VarList        {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"VarList",3,$1,$2,$3);}
  | ParamDec                            {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"VarList",1,$1);}
  ;
ParamDec : Specifier VarDec             {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"ParamDec",2,$1,$2);}
  ;

/*Statements*/
CompSt : LC DefList StmtList RC         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"CompSt",4,$1,$2,$3,$4);}
  | error RC                            {synError = 1;}
  ;
StmtList : /*empty*/                    {$$ = NULL;}
  | Stmt StmtList                       {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"StmtList",2,$1,$2);}
  ;
Stmt : Exp SEMI                         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Stmt",2,$1,$2);}
  | CompSt                              {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Stmt",1,$1);}
  | RETURN Exp SEMI                     {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Stmt",3,$1,$2,$3);}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Stmt",5,$1,$2,$3,$4,$5);}
  | IF LP Exp RP Stmt ELSE Stmt         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Stmt",7,$1,$2,$3,$4,$5,$6,$7);}
  | WHILE LP Exp RP Stmt                {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Stmt",5,$1,$2,$3,$4,$5);}
  | error SEMI                          {synError = 1;}
  ;

/*Local Definitions*/
DefList : /*empty*/                     {$$ = NULL;}
  | Def DefList                         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"DefList",2,$1,$2);}
  ;
Def : Specifier DecList SEMI           {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Def",3,$1,$2,$3);}
  ;
DecList : Dec                           {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"DecList",1,$1);}
  | Dec COMMA DecList                   {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"DecList",3,$1,$2,$3);}
  ;
Dec : VarDec                            {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Dec",1,$1);}
  | VarDec ASSIGNOP Exp                 {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Dec",3,$1,$2,$3);}
  ;

/*Expressions*/
Exp : Exp ASSIGNOP Exp                  {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp AND Exp                         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp OR Exp                          {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp RELOP Exp                       {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp PLUS Exp                        {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp SUB Exp                         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp STAR Exp                        {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp DIV Exp                         {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | LP Exp RP                           {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | SUB Exp                             {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",2,$1,$2);}
  | NOT Exp                             {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",2,$1,$2);}
  | ID LP Args RP                       {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",4,$1,$2,$3,$4);}
  | ID LP RP                            {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp LB Exp RB                       {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",4,$1,$2,$3,$4);}
  | Exp DOT ID                          {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",3,$1,$2,$3);}
  | ID                                  {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",1,$1);}
  | INT                                 {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",1,$1);}
  | FLOAT                               {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Exp",1,$1);}
  | NONHEX                              {$$ = NULL;}
  | NONOCT                              {$$ = NULL;}
  ;
Args : Exp COMMA Args                   {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Args",3,$1,$2,$3);}
  | Exp                                 {$$ = newNonterminatorToken(NOT_TOKEN,@$.first_line,"Args",1,$1);}
  ;

/*Error*/ 
%%

yyerror(char *msg){
    fprintf(stdout,"Error type B at Line %d: %s.\n",yylineno,msg);
}
