#include "intermediate.h"

typedef struct IntStack_* IntStack;
struct IntStack_{
    IntStack next;
    int val;
} IntStack_;


void translateInstr();
void translateNormalInstr(InterCode code);
void translateInstrArr(InterCode code);
void translateCond(InterCode code);
void translateFunc(InterCode code);
void translateRet(InterCode code);
void translateCALL(InterCode args,int argsCnt,InterCode call);
void translateParam(InterCode param,int paramCnt);
void initForInst(FILE *ptr);
void initCodeFile(FILE* ptr);
char* ensure(char* x,bool isKill);
char* allocate(char* x);
void spill(RegDescriptor regDes,char* reg);
bool isArrBlock(InterCode code);

void divideBlock(InterCode code);
void addLineNoForBlock(char* varName,int lineNo);
VarDescriptor searchVarInfoForBlock(char* varName);
VarDescriptor addVarUseInfoForBlock(char* varName);
void pushIntStack(int val);
int popIntStack();
