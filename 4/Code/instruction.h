#include "semantic.h"

void addLineNo(char* varName,int lineNo);
VarDescriptor searchVarInfo(char* varName);
VarDescriptor addVarUseInfo(char* varName);
void translateInstr();
void initForInst(FILE *ptr);
char* ensure(char* x);
char* allocate(char* x);