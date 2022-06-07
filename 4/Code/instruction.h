#include "intermediate.h"

void addLineNo(char* varName,int lineNo);
VarDescriptor searchVarInfo(char* varName);
VarDescriptor addVarUseInfo(char* varName);
void translateInstr();
void translateNormalInstr(InterCode code);
void translateInstrArr(InterCode code);
bool isArrBlock(InterCode code);
void initForInst(FILE *ptr);
void initCodeFile(FILE* ptr);
char* ensure(char* x);
char* allocate(char* x);
void spill(RegDescriptor regDes);