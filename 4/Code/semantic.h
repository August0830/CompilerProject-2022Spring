#include "node.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum ErrorType_ {
    UNDEF_VAR = 1, // Undefined variable.
    UNDEF_FUNC, // Undefined function.
    REDEF_VAR, // Redefined variable.
    REDEF_FUNC, // Redefined function. 
    TYPE_MISMATCHED_ASSIGN,  //  Type mismatched for assignment.
    LEFT_VAR_ASSIGN, // The left-hand side of an assignment must be a variable.
    TYPE_MISMATCHED_OP, // Type mismatched for operands.
    TYPE_MISMATCHED_RETURN, // Type mismatched for return. 
    FUNC_ARGS_NOT_APP, //  Function  is not applicable for arguments.
    NOT_ARRAY,// variable is not an array.
    NOT_FUNC, //  is not a function.
    ARR_INDEX_NOT_INT, // is not an integer.
    ILLEGAL_DOT, //Illegal use of ".".
    NOTEXIST_FIELD, // Non-existent field.
    REDEF_FIELD, //Redefined field.
    DUPLICATED_STRUCT_NAME, //Duplicated name. 
    UNDEF_STRUCT // Undefined structure.
} ErrorType;

typedef enum _TypeKind {
    BASIC, // INT FLOAT
    T_ID,
    T_STRUCT, 
    FUNC,
    ARRAY
} TypeKind;

typedef enum _BasicType{
    B_INT,
    B_FLOAT
} BasicType;

typedef struct _StructHead_{
    struct _StructHead_* prev;
    struct _StructHead_* next;
    struct _FieldList_* structFieldList;
    char* structName;
} StructHead_;

typedef StructHead_* StructHead;

typedef struct _FuncHead_{
    struct _FuncHead_* prev;
    struct _FuncHead_* next;
    struct _FieldList_* funcFieldList;
    char* funcName;
    struct _Type_* returnType;
} FuncHead_;

typedef FuncHead_* FuncHead;

typedef struct _ArrayType{
    struct _Type_* elemType;
    int size;
} ArrayInfo;

typedef struct _Type_{
    TypeKind kind;
    union {
        BasicType basicType;
        StructHead structTypeInfo;
        FuncHead funcTypeInfo;
        ArrayInfo arrayInfo;
    }info;
} Type_;

typedef Type_* Type;

typedef struct Operand_* Operand;
struct Operand_ {
    enum {VARIABLE,CONSTANT,ADDR,GETADDR,VALOFADDR} kind;
    union{
        char* var_name;
        int val_int;
        float val_float;
    } u;
};

typedef struct _Reg{
    char* codeVarId;
    char* interVarId;
    struct _Reg* next;
    int size;
    bool isArr;
    bool isAddr;
    Operand op;
} _Reg;

typedef struct _Reg* Reg;

typedef union {
        int iVal;
        float fVal; //for both basic type when kind is BASIC
        struct _FieldList_* structField;
        struct _FuncVarInfo_* funcInfo;
        ArrayInfo arrayInfo;
} Val;

typedef struct _FuncVarInfo_{
    struct _FieldList_* funcField;
    Val returnVal;
} FuncVarInfo_;

typedef FuncVarInfo_* FuncVarInfo;

typedef struct _SymbolItem_{
    Type type;
    Val val;
    char* name;
    Reg reg;
    struct _SymbolItem_* next;
}SymbolItem_;

typedef SymbolItem_* SymbolItem;

typedef struct _FieldList_{
    SymbolItem fieldVar;
    struct _FieldList_* nextField;
} _FieldList;

typedef _FieldList* FieldList;

typedef struct VarDescriptor_* VarDescriptor;
struct VarDescriptor_{
    VarDescriptor left;
    VarDescriptor right;
    union{
        char* varName;
        int lineNo;
    } u;
} VarDescriptor_;

typedef struct RegDescriptor_* RegDescriptor;
struct RegDescriptor_{
    bool free;
    char* varStore;
} RegDescriptor_;

typedef struct InterCode_* InterCode;
struct InterCode_ {
    enum {
        ASSIGN, 
        PLUS,
        SUB,
        STAR,
        DIV,
        AND,
        OR,
        RELOP,
        NOT,
        MINUS,
        DEC,
        COND,
        LABEL,
        JMP,
        FUNCINFO,
        PARAM,
        ARG,
        CALL,  
        RETURN
    } kind;
    union{
        struct {Operand right,left;} assign;
        struct {Operand res,op1,op2;} binop;
        struct {Operand res,op1;} singleop;
        struct {InterCode trueLabel,falseLabel,cond;} condition;
        struct {InterCode compst; char* funcName;} func;
        struct {Operand arr; int size;} arr;
        struct {Operand op1,op2; char* oper; InterCode label;} cond;
        struct {char* labelName;} label;
        struct {InterCode targetCode; char* targetLabel;} jump;
        struct {char* funcName; int argsCnt;} funInfo;
        struct {char* varName;} par;
        struct {char* func;Operand res;} callInfo;
    } u;
    InterCode prev,next;
    bool isBlockStarter;
} InterCode_;

void pError(ErrorType type, int line,char* msg);
char* newString(char* src);
unsigned int generateHashCode(char* name);
void init();
void traverseTree(Node* node);
void prepareFunc(FILE* fp);

void addFunc(FuncHead f);
FuncHead findFunc(char* name);
StructHead findStruct(char* name);
bool isMatchedFieldList(FieldList f1,FieldList f2);
bool isSameType(Type t1, Type t2, bool canTransformed);
bool isOperand(Type t);
bool isBasicType(Type t, BasicType basic);
void addSymbol(SymbolItem item,int lineNum);
SymbolItem findSymbolInTable(char* name);

Type Specifier(Node* node);
void FunDec(Node* node,Type returnType);
void ExtDef(Node* node);
void ExtDefList(Node *node);
FieldList DefList(Node* node);
FieldList Def(Node* node);
FieldList DecList(Node* node, Type type);
SymbolItem Dec(Node *node, Type type);
void ExtDecList(Node* node, Type type);
void CompSt(Node* node, Type returnType);
SymbolItem VarDec(Node *node, Type type);
void StmtList(Node* node,Type returnType);
void Stmt(Node *node, Type returnType);
FieldList Args(Node* node);
SymbolItem Exp(Node *node);
SymbolItem ParamDec(Node* node);
FieldList VarList(Node* node);
StructHead StructSpecifier(Node* node);