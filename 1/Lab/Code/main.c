#include <unistd.h>
#include <stdio.h>
#include "node.h"
#include "syntax.tab.h"

extern FILE* yyin;
extern ptrNode root;
extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE*);

unsigned int lexError = 0;
unsigned int synError = 0;

int main(int argc,char**argv){
    if(argc<=1){
        yyparse();
        return 1;
    } 
    FILE* f = fopen(argv[1],"r");
    if(!f)
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if(lexError == 0 && synError == 0){
        
        printNodeTree(root,0);
    }
    deleteNode(root);
    return 0;
}