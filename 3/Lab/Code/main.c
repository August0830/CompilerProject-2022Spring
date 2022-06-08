#include <unistd.h>
#include <stdio.h>
#include "intermediate.h"
// #include "node.h"
#include "syntax.tab.h"


extern FILE* yyin;
extern ptrNode root;
extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE*);
extern bool hasStruct;

unsigned int lexError = 0;
unsigned int synError = 0;


int main(int argc,char**argv){
    if(argc<=1){
        yyparse();
        return 1;
    } 
    else if(argc<2){
        return 2;
    }
    FILE* f = fopen(argv[1],"r");
    FILE* output = fopen(argv[2],"w");
    if(!f)
    {
        perror(argv[1]);
        return 1;
    }
    if(!output)
    {
        perror(argv[2]);
        return 2;
    }
    yyrestart(f);
    yyparse();
    init();
    prepareFunc(output);
    prepareIntermediate(output);
    if(lexError == 0 && synError == 0){        
        printNodeTree(root,0);
        traverseTree(root);
        if(!hasStruct)
            generateInterCode(root);
        else
            printf("Cannot translate: Code contains variables or parameters of structure type.\n");
    }
    deleteNode(root);
    fclose(f);
    fclose(output);
    return 0;
}