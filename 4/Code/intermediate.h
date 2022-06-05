#include "semantic.h"
char* newTempName();
char* newVarName();
char* newLabel();
Reg newReg(char* codeVarId,char* interVarId);
void copyReg(Reg* dst, Reg* src);
Reg calOffset(Node* Exp, char* offset,Type* type,int* constant);
SymbolItem isArrayExp(Node* exp);
SymbolItem isArrayId(char* id);
void copyArr(char* dst, char* src);
Reg reverse(Reg head);
void addCodes(InterCode ic);
Operand copyOperand(Operand op);
void addLineNo(char* varName,int lineNo);
VarDescriptor searchVarInfo(char* varName);
VarDescriptor addVarUseInfo(char* varName);

void prepareIntermediate(FILE* ptr);
void generateInterCode(Node* node);
void translate_Exp_Func(Node* Exp, Reg place);
void translate_Args(Node* Args,Reg argList);
void translate_Cond(Node* Exp,InterCode labelTrue, InterCode labelFalse);
void translate_Stmt(Node* Stmt);
void translate_VarDec(Node* varDec, Reg place);
void translate_DecList(Node* decList);
void translate_DefList(Node* defList);
void translate_CompSt(Node* compSt);
void translate_Exp(Node* Exp, Reg place);
void translate_FunDec(Node* funDec);