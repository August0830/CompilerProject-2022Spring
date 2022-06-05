#include "semantic.h"

#define HASHTABLE_SIZE 0x3fff
int anonymousCnt;
int anonymousStruct;
SymbolItem symbolTable[HASHTABLE_SIZE];
FuncHead funcList;
FuncHead funcPtr;
StructHead structList;
StructHead structPtr;
int isInStruct;
int isInArrayDef;

bool hasStruct;

void init()
{
    for (int i = 0; i < HASHTABLE_SIZE; ++i)
        symbolTable[i] = NULL;
    funcList = (FuncHead)malloc(sizeof(FuncHead_));
    assert(funcList != NULL);
    funcList->prev = NULL;
    funcList->next = NULL;
    funcList->funcName = newString("funcHead");
    funcPtr = funcList;

    structList = (StructHead)malloc(sizeof(StructHead_));
    assert(structList != NULL);
    structList->prev = NULL;
    structList->next = NULL;
    structList->structName = newString("structHead");
    structPtr = structList;
    anonymousCnt = 0;
    anonymousStruct = 0;
    isInStruct = 0;
    isInArrayDef = 0;
    hasStruct = false;
}

void pError(ErrorType type, int line, char *msg)
{
    printf("Error type %d at Line %d: %s\n", type, line, msg);
}

char *newString(char *src)
{
    if (src == NULL)
        return NULL;
    int len = strlen(src) + 1;
    char *str = (char *)malloc(len * sizeof(char));
    assert(str != NULL);
    strncpy(str, src, len);
    return str;
}

unsigned int generateHashCode(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~0x3fff)
            val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

void traverseTree(Node *node)
{
    if (node == NULL)
        return;
    if (!strcmp(node->child->name, "ExtDefList"))
        ExtDefList(node->child);
    else
        traverseTree(node->child);
    traverseTree(node->next);
}

//ExtDefList : ExtDef ExtDefList
void ExtDefList(Node *node)
{
    assert(node != NULL);
    Node *child = node->child;
    if (!strcmp(child->name, "ExtDef"))
    {
        ExtDef(child);
        if (child->next != NULL && !strcmp(child->next->name, "ExtDefList"))
            ExtDefList(child->next);
    }
}

Type Specifier(Node *node)
{
    assert(node != NULL);
    Type type = (Type)malloc(sizeof(Type_));
    assert(type != NULL);
    //Specifier : TYPE
    if (!strcmp(node->child->name, "TYPE"))
    {
        Node *typenode = node->child;
        if (!strcmp(typenode->val.val, "int"))
        {
            type->kind = BASIC;
            type->info.basicType = B_INT;
        }
        else if (!strcmp(typenode->val.val, "float"))
        {
            type->kind = BASIC;
            type->info.basicType = B_FLOAT;
        }
    }
    //Specifier : StructSpecifier
    else if (!strcmp(node->child->name, "StructSpecifier"))
    {
        //type structSpecifier
        type->kind = T_STRUCT;
        type->info.structTypeInfo = StructSpecifier(node->child);
        hasStruct = true;
    }
    return type;
}
// StructSpecifier : STRUCT OptTag LC DefList RC
// StructSpecifier : STRUCT Tag
// OptTag : ID
// Tag : ID
StructHead StructSpecifier(Node *node)
{
    assert(node != NULL);
    //step 1: get info
    StructHead newStruct = (StructHead)malloc(sizeof(StructHead_));
    isInStruct--;
    Node *child = node->child;
    Node *tag = child->next;
    //pay attention to empty
    if(!strcmp(tag->name,"LC"))
    {
        char name[10] = {0};
        sprintf(name, "anonymousStruct %d", anonymousStruct);
        newStruct->structName = newString(name);
        anonymousStruct++;
    }
    // OptTag : ID
    // Tag : ID
    else{
        if (!strcmp(tag->child->name, "ID"))
            newStruct->structName = newString(tag->child->val.val);
    }
    
    // StructSpecifier : STRUCT OptTag LC DefList RC
    // define
    if (!strcmp(child->next->name, "OptTag"))
    {
        Node *defList = child->next->next->next;
        
        newStruct->structFieldList = DefList(defList);

        //step 2: check redef
        if (findStruct(newStruct->structName) != NULL)
        {
            char msg[100] = {0};
            sprintf(msg, "Duplicated name \"%s\".", newStruct->structName);
            pError(DUPLICATED_STRUCT_NAME, node->lineNum, msg);
            return NULL;
        }

        //step 3: add to struct table
        newStruct->prev = structPtr;
        structPtr->next = newStruct;
        structPtr = structPtr->next;
    }
    // StructSpecifier : STRUCT Tag
    // use
    else if (!strcmp(child->next->name, "Tag"))
    {
        char *tagName = newString(tag->child->val.val);
        StructHead tmp  = findStruct(tagName);
        if (tmp == NULL)
        {
            char msg[100] = {0};
            sprintf(msg, "Undefined structure \"%s\".", tagName);
            pError(UNDEF_STRUCT, node->lineNum, msg);
        }
        else
        {
            free(newStruct);
            newStruct = tmp;
        }
    }
    // StructSpecifier : STRUCT LC DefList RC
    else if (!strcmp(child->next->name, "LC"))
    {
        Node *defList = child->next->next;
        newStruct->structFieldList = DefList(defList);

        //step 2: check redef
        if (findStruct(newStruct->structName) != NULL)
        {
            char msg[100] = {0};
            sprintf(msg, "Duplicated name \"%s\".", newStruct->structName);
            pError(DUPLICATED_STRUCT_NAME, node->lineNum, msg);
            return NULL;
        }

        //step 3: add to struct table
        newStruct->prev = structPtr;
        structPtr->next = newStruct;
        structPtr = structPtr->next;
    }
    isInStruct++; //use serialized property
    return newStruct;
}

StructHead findStruct(char *name)
{
    StructHead fptr = structList->next;
    while (fptr != NULL)
    {
        if (!strcmp(fptr->structName, name))
            return fptr;
        fptr = fptr->next;
    }
    return NULL;
}

//FunDec : ID LP VarList RP
//FunDec : ID LP RP
void FunDec(Node *node, Type returnType)
{
    //step one: distract func info: return type;param list;name

    assert(returnType != NULL);
    assert(node != NULL);
    FuncHead funcInfo = (FuncHead)malloc(sizeof(FuncHead_));
    assert(funcInfo != NULL);
    funcInfo->returnType = returnType;
    funcInfo->funcName = NULL;
    funcInfo->next = NULL;
    funcInfo->prev = NULL;
    if (!strcmp(node->child->name, "ID"))
    {
        funcInfo->funcName = newString(node->child->val.val);
        //step two: check redefine
        if (findFunc(funcInfo->funcName) != NULL)
        {
            char msg[100] = {0};
            sprintf(msg, "Redefined function \"%s\"", funcInfo->funcName);
            pError(REDEF_FUNC, node->lineNum, msg);
            //return; no return, still need to load variable
        }
    }
    // FunDec : ID LP VarList RP
    if (!strcmp(node->child->next->next->name, "VarList"))
    {
        funcInfo->funcFieldList = VarList(node->child->next->next);
    }
    //FunDec : ID LP RP
    else
    {
        funcInfo->funcFieldList = NULL;
    }

    //step three: add it to funcList
    funcInfo->prev = funcPtr;
    funcPtr->next = funcInfo;
    funcPtr = funcPtr->next;
    //printf(funcPtr->funcName);
}

FieldList VarList(Node *node)
{
    assert(node != NULL);
    FieldList varList = (FieldList)malloc(sizeof(_FieldList));
    if (!strcmp(node->child->name, "ParamDec"))
    {
        varList->fieldVar = ParamDec(node->child);
        //VarList : ParamDec COMMA VarList
        if (node->child->next != NULL && !strcmp(node->child->next->next->name, "VarList"))
        {
            varList->nextField = VarList(node->child->next->next);
        }
        else //VarList : ParamDec
            varList->nextField = NULL;
    }
    return varList;
}

SymbolItem ParamDec(Node *node)
{
    assert(node != NULL);
    //ParamDec : Specifier VarDec
    Type type = Specifier(node->child);
    return VarDec(node->child->next, type);
}

void ExtDef(Node *node)
{
    assert(node != NULL);
    if (!strcmp(node->child->name, "Specifier"))
    {
        Type type = Specifier(node->child);
        if (!strcmp(node->child->next->name, "FunDec"))
        {
            FunDec(node->child->next, type);
            //ExtDef : Specifier FunDec CompSt
            if (node->child->next->next != NULL && !strcmp(node->child->next->next->name, "CompSt"))
            {
                CompSt(node->child->next->next, type);
            }
        }
        //ExtDef : Specifier ExtDecList SEMI
        else if (!strcmp(node->child->next->name, "ExtDecList"))
        {
            ExtDecList(node->child->next, type);
        }
    }
}

void CompSt(Node *node, Type returnType)
{
    assert(node != NULL);
    //CompSt : LC DefList StmtList RC
    Node *child = node->child;
    if (child->next != NULL)
    {
        if (!strcmp(child->next->name, "DefList"))
        {
            DefList(child->next);
            if (child->next->next != NULL && !strcmp(child->next->next->name, "StmtList"))
                StmtList(child->next->next, returnType);
        }
        //CompSt : LC DefList StmtList RC
        else if (!strcmp(child->next->name, "StmtList"))
            StmtList(child->next, returnType);
    }
}

//DefList : Def DefList
FieldList DefList(Node *node)
{
    assert(node != NULL);
    if (!strcmp(node->child->name, "Def"))
    {
        FieldList field = (FieldList)malloc(sizeof(_FieldList));
        field = Def(node->child);
        FieldList next = field;
        while (next->nextField != NULL)
            next = next->nextField;
        if (node->child->next != NULL && !strcmp(node->child->next->name, "DefList"))
            next->nextField = DefList(node->child->next);
        return field;
    }
    return NULL;
}

//Def : Specifier DecList SEMI
FieldList Def(Node *node)
{
    assert(node != NULL);
    if (!strcmp(node->child->name, "Specifier"))
    {
        Type type = Specifier(node->child);
        if (!strcmp(node->child->next->name, "DecList"))
        {
            return DecList(node->child->next, type);
        }
    }
    return NULL;
}

FieldList DecList(Node *node, Type type)
{
    //DecList : Dec
    //DecList : Dec COMMA DecList
    assert(node != NULL);
    Node *child = node->child;
    if (!strcmp(child->name, "Dec"))
    {
        FieldList field = (FieldList)malloc(sizeof(_FieldList));
        field->fieldVar = Dec(child, type);
        if (child->next != NULL && !strcmp(child->next->next->name, "DecList"))
            field->nextField = DecList(child->next->next, type);
        else
            field->nextField = NULL;
        return field;
    }
    return NULL;
}

SymbolItem Dec(Node *node, Type type)
{
    assert(node != NULL);
    Node *child = node->child;
    //Dec : VarDec
    if (!strcmp(child->name, "VarDec"))
    {
        SymbolItem var = VarDec(child, type);
        //Dec : VarDec ASSIGNOP Exp
        if (child->next != NULL && !strcmp(child->next->name, "ASSIGNOP") && child->next->next != NULL && !strcmp(child->next->next->name, "Exp"))
        {
            if(isInStruct<0)
            {
                char msg[100]={0};
                sprintf(msg,"Cannot initialize field when define struct");
                pError(REDEF_FIELD,node->lineNum,msg);
                return NULL;
            }
            SymbolItem val = Exp(child->next->next);
            if (val == NULL)
                return NULL;
            if (isSameType(var->type, val->type,false))
            {
                var->val = val->val;
            }
            else
            {
                char msg[100] = {0};
                sprintf(msg, "Type mismatched for assignment.");
                pError(TYPE_MISMATCHED_ASSIGN, node->lineNum, msg);
            }
        }
        return var;
    }
    else
        return NULL;
}

void ExtDecList(Node *node, Type type)
{
    assert(node != NULL);
    //ExtDecList :  VarDec COMMA ExtDecList
    if (!strcmp(node->child->name, "VarDec"))
    {
        VarDec(node->child, type);
        if (node->child->next != NULL && !strcmp(node->child->next->next->name, "ExtDecList"))
        {
            ExtDecList(node->child->next->next, type);
        }
        //ExtDecList :  VarDec
    }
}

SymbolItem VarDec(Node *node, Type type)
{
    assert(node != NULL);
    //step 1 get info
    SymbolItem var = (SymbolItem)malloc(sizeof(SymbolItem_));
    var->type = type;
    var->next = NULL;
    var->reg = NULL;
    Node *child = node->child;

    //VarDec : VarDec LB INT RB
    if (!strcmp(child->name, "VarDec"))
    {
        isInArrayDef++;
        
        // type should be the elem type
        Type arrayType = (Type)malloc(sizeof(Type_));
        arrayType->kind = ARRAY;
        arrayType->info.arrayInfo.elemType = type;
        Node *sizeNode = child->next->next;
        if (!strcmp(sizeNode->name, "INT"))
        {
            int elemSize = 4;
            if(type->kind == ARRAY)
                elemSize = type->info.arrayInfo.size;
            arrayType->info.arrayInfo.size = sizeNode->val.iVal * elemSize;
        }
        else
            arrayType->info.arrayInfo.size = 0;
        var = VarDec(child, arrayType);
    }
    else if (!strcmp(child->name, "ID")) //VarDec : ID
    {
        var->name = newString(child->val.val);
        // if(isInArrayDef>0)//you are basic type
        // {
        // }
        // else 
        if (isInArrayDef==0 && var->type->kind != T_STRUCT)
            var->type->kind = T_ID;
        
    }
    // step 2 add to symboltable
    if (isInArrayDef==0)
        addSymbol(var, node->lineNum);
    else
        isInArrayDef--;
    return var;
}

SymbolItem Exp(Node *node)
{
    assert(node != NULL);
    Node *child = node->child;

    if (!strcmp(child->name, "ID"))
    {
        SymbolItem item = findSymbolInTable(child->val.val);
        // Exp : ID
        if (child->next == NULL)
        {
            //should be define before
            if (item == NULL)
            {
                char msg[100] = {0};
                sprintf(msg, "Undefined variable \"%s\".", child->val.val);
                pError(UNDEF_VAR, node->lineNum, msg);
            }
            return item;
        }
        //Exp : ID LP Args RP
        //Exp : ID LP RP
        else
        {
            if (!strcmp(child->next->name, "LP")) //function
            {
                //check func
                FuncHead funcInfo = findFunc(child->val.val);
                if (funcInfo == NULL)
                {
                    if (item != NULL) //not in func table but in symbol table
                    {
                        char msg[100] = {0};
                        sprintf(msg, "\"%s\" is not a function.", item->name);
                        pError(NOT_FUNC, node->lineNum, msg);
                        return NULL;
                    }
                    else
                    {
                        char msg[100] = {0};
                        sprintf(msg, "Undefined function \"%s\".", child->val.val);
                        pError(UNDEF_FUNC, node->lineNum, msg);
                        return NULL;
                    }
                }
                //check param
                //Exp : ID LP Args RP
                if (!strcmp(child->next->next->name, "Args"))
                {
                    FieldList args = Args(child->next->next);
                    if (!isMatchedFieldList(args, funcInfo->funcFieldList))
                    {
                        char msg[100] = {0};
                        sprintf(msg, "Function \"%s\" is not applicable for arguments.", funcInfo->funcName);
                        pError(FUNC_ARGS_NOT_APP, node->lineNum, msg);
                        return NULL;
                    }
                }
                //Exp : ID LP RP
                else
                {
                    if (funcInfo->funcFieldList != NULL)
                    {
                        char msg[100] = {0};
                        sprintf(msg, "Function \"%s\" is not applicable for arguments.", funcInfo->funcName);
                        pError(FUNC_ARGS_NOT_APP, node->lineNum, msg);
                        return NULL;
                    }
                }
                SymbolItem item = (SymbolItem)malloc(sizeof(SymbolItem_));
                char returnVal[20] = {0};
                sprintf(returnVal, "ret of \"%s\"", funcInfo->funcName);
                item->name = newString(returnVal);
                item->next = NULL;
                item->type = funcInfo->returnType;
                item->reg = NULL;
                return item;
            }
        }
    }
    // Exp : INT
    else if (!strcmp(child->name, "INT"))
    {
        SymbolItem item = (SymbolItem)malloc(sizeof(SymbolItem_));
        Type type = (Type)malloc(sizeof(Type_));
        type->kind = BASIC;
        type->info.basicType = B_INT;
        item->type = type;
        item->val.iVal = child->val.iVal;
        char name[10] = {0};
        sprintf(name, "int%d %d", anonymousCnt, item->val.iVal);
        item->name = newString(name);
        anonymousCnt++;
        item->next = NULL;
        return item;
    }
    // Exp : FLOAT
    else if (!strcmp(child->name, "FLOAT"))
    {
        SymbolItem item = (SymbolItem)malloc(sizeof(SymbolItem_));
        Type type = (Type)malloc(sizeof(Type_));
        type->kind = BASIC;
        type->info.basicType = B_FLOAT;
        item->type = type;
        item->val.fVal = atof(child->val.val);
        char name[10] = {0};
        sprintf(name, "float %f", item->val.fVal);
        item->name = newString(name);
        item->next = NULL;
        return item;
    }
    else if (!strcmp(child->name, "Exp"))
    {
        SymbolItem e1 = Exp(child);
        if (e1 == NULL)
            return NULL;
        //Exp : Exp ASSIGNOP Exp
        if (!strcmp(child->next->name, "ASSIGNOP"))
        {
            SymbolItem e2 = Exp(child->next->next);
            if(e2 == NULL)
                return NULL;
            if (e1->type->kind == BASIC)
            {
                char msg[100] = {0};
                sprintf(msg, "The left-hand side of an assignment must be a variable.");
                pError(LEFT_VAR_ASSIGN, node->lineNum, msg);
                return NULL;
            }
            else if (e2 == NULL || !isSameType(e1->type, e2->type,false))
            {
                char msg[100] = {0};
                sprintf(msg, "Type mismatched for assignment.");
                pError(TYPE_MISMATCHED_ASSIGN, node->lineNum, msg);
                return NULL;
            }
            else //matched
            {
                e1->val = e2->val;
                return e1;
            }
            
        }
        //Exp : Exp AND Exp
        //      Exp OR Exp
        //      Exp RELOP Exp
        //      Exp PLUS Exp
        //      Exp SUB Exp
        //      Exp STAR Exp
        //      Exp DIV Exp
        else if (!strcmp(child->next->name, "AND") || !strcmp(child->next->name, "OR") || !strcmp(child->next->name, "RELOP") || !strcmp(child->next->name, "PLUS") || !strcmp(child->next->name, "SUB") || !strcmp(child->next->name, "STAR") || !strcmp(child->next->name, "DIV"))
        {
            SymbolItem e2 = Exp(child->next->next);
            if (e2 == NULL)
                return NULL;
            bool b1 = isOperand(e1->type);
            bool b2 = isOperand(e2->type);
            bool b3 = isSameType(e1->type, e2->type,true);
            if ( !b1 || !b2 || !b3)
            {
                char msg[100] = {0};
                sprintf(msg, "Type mismatched for operands.");
                pError(TYPE_MISMATCHED_OP, node->lineNum, msg);
                return NULL;
            }
            SymbolItem item = (SymbolItem)malloc(sizeof(SymbolItem_));
            char name[20] = {0};
            sprintf(name,"%s at %d", child->next->name, node->lineNum);               
            item->name = newString(name);
            Type resType = (Type)malloc(sizeof(Type_));
            resType->kind = BASIC;
            if(!strcmp(child->next->name, "AND") || !strcmp(child->next->name, "OR") || !strcmp(child->next->name, "RELOP"))
            {
                resType->info.basicType = B_INT;
            }
            else
                resType->info = e1->type->info;
            item->type = resType;
            item->val = e1->val;
            item->next = NULL;
            item->reg = NULL;
            //actually it should be a real caculation but this time I will ignore it
            return item;
        }
        //Exp : Exp LB Exp RB
        else if (!strcmp(child->next->name, "LB"))
        {
            if (e1->type->kind != ARRAY)
            {
                char msg[100] = {0};
                sprintf(msg, "\"%s\" is not an array.", e1->name);
                pError(NOT_ARRAY, node->lineNum, msg);
                return NULL;
            }
            SymbolItem index = Exp(child->next->next);
            if (!isBasicType(index->type,B_INT))
            //(index->type->kind != BASIC || index->type->info.basicType != B_INT)
            {
                char msg[100] = {0};
                sprintf(msg, "\"%s\" is not an integer.", index->name);
                pError(ARR_INDEX_NOT_INT, node->lineNum, msg);
                return NULL;
            }
            SymbolItem item = (SymbolItem)malloc(sizeof(SymbolItem_));
            char name[30] ={0};
            sprintf(name, "%s elem", e1->name);
            item->name = newString(name);
            item->next = NULL;
            item->type = e1->type->info.arrayInfo.elemType;
            if(item->type->kind == BASIC)
            {
                item->type->kind = T_ID;
            }
            item->reg = NULL;
            return item;
        }
        //Exp : Exp DOT ID
        else if (!strcmp(child->next->name, "DOT"))
        {
            if (e1->type->kind != T_STRUCT)
            {
                char msg[100] = {0};
                sprintf(msg, "Illegal use of \".\"");
                pError(ILLEGAL_DOT, node->lineNum, msg);
                return NULL;
            }
            char *id = newString(child->next->next->val.val);
            FieldList fieldInfo = e1->type->info.structTypeInfo->structFieldList;
            while (fieldInfo != NULL)
            {
                if (fieldInfo->fieldVar!=NULL && !strcmp(fieldInfo->fieldVar->name, id))
                {
                    SymbolItem item = (SymbolItem)malloc(sizeof(SymbolItem_));
                    char name[30] = {0};
                    sprintf(name, "%s.%s", e1->name, id);
                    item->name = newString(name);
                    item->next = NULL;
                    item->type = fieldInfo->fieldVar->type;
                    item->reg = NULL;
                    return item;
                }
                fieldInfo = fieldInfo->nextField;
            }
            char msg[100] = {0};
            sprintf(msg, "Non-existent field \"%s\".", id);
            pError(NOTEXIST_FIELD, node->lineNum, msg);
            return NULL;
        }
    }
    //Exp : SUB Exp
    //Exp : NOT Exp
    else if (!strcmp(child->name, "SUB") || !strcmp(child->name, "NOT"))
    {
        return Exp(child->next);
        //regardless of real caculation
    }
    //Exp : LP Exp RP
    else if (!strcmp(child->name, "LP") && child->next != NULL)
    {
        return Exp(child->next);
    }

}

FieldList Args(Node *node)
{
    FieldList args = (FieldList)malloc(sizeof(_FieldList));
    //Args : Exp COMMA Args
    args->fieldVar = Exp(node->child);
    if (args->fieldVar == NULL)
        return NULL;
    if (node->child->next != NULL && !strcmp(node->child->next->next->name, "Args"))
    {
        args->nextField = Args(node->child->next->next);
    }
    else //Args : Exp
        args->nextField = NULL;
    return args;
}

FuncHead findFunc(char *name)
{
    FuncHead fptr = funcList;
    while (fptr != NULL)
    {
        if (!strcmp(fptr->funcName, name))
            return fptr;
        fptr = fptr->next;
    }
    return NULL;
}

bool isMatchedFieldList(FieldList f1, FieldList f2)
{
    while (f1 != NULL && f2 != NULL)
    {
        if (!isSameType(f1->fieldVar->type, f2->fieldVar->type,false))
            return false;
        f1 = f1->nextField;
        f2 = f2->nextField;
    }
    if (f1 == NULL && f2 == NULL)
        return true;
    else
        return false;
}

bool isSameType(Type t1, Type t2, bool canTransformed)
{
    if (t1 == t2)
        return true;
    if(t1 == NULL || t2 == NULL)
        return false;
    if (t1->kind == T_STRUCT && t2->kind == T_STRUCT)
    {
        FieldList f1 = t1->info.structTypeInfo->structFieldList;
        FieldList f2 = t2->info.structTypeInfo->structFieldList;
        while (f1 != NULL && f2 != NULL)
        {
            if (!isSameType(f1->fieldVar->type, f2->fieldVar->type,false))
                return false;
            f1 = f1->nextField;
            f2 = f2->nextField;
        }
        if (f1 == NULL && f2 == NULL)
            return true;
        else
            return false;
    }
    else if ((t1->kind == T_ID && t2->kind == BASIC) || (t1->kind == BASIC && t2->kind == T_ID) || (t1->kind == BASIC && t2->kind == BASIC) || (t1->kind == T_ID && t2->kind == T_ID)) //for assignment
    // basic type does not point to same item
    {
        //int can match float
        bool same = t1->info.basicType == t2->info.basicType;
        if(canTransformed)
        {
            same = same || (t1->info.basicType == B_FLOAT && t2->info.basicType == B_INT)
        || (t1->info.basicType == B_INT && t2->info.basicType == B_FLOAT);
        }
        return same;
    }
    else if (t1->kind == ARRAY && t2->kind == ARRAY)
    {
        return isSameType(t1->info.arrayInfo.elemType, t2->info.arrayInfo.elemType,false);
    }
    else
        return false;
}

bool isOperand(Type t)
{
    if(t->kind == BASIC)
        return true;
    if(t->kind == T_ID)
        return t->info.basicType == B_INT || t->info.basicType == B_FLOAT;
    else 
        return false;
}

bool isBasicType(Type t, BasicType basic)
{
    if(t->kind == BASIC && t->info.basicType == basic)
        return true;
    else if(t->kind == T_ID && t->info.basicType == basic)
        return true;
    else 
        return false;
}

SymbolItem findSymbolInTable(char *name)
{
    unsigned int hashCode = generateHashCode(name);
    if (symbolTable[hashCode] == NULL)
        return NULL;
    SymbolItem item = symbolTable[hashCode];
    while (item != NULL)
    {
        if (!strcmp(item->name, name))
            return item;
        item = item->next;
    }
    return NULL;
}

void addSymbol(SymbolItem item, int lineNum)
{
    assert(item != NULL && item->name != NULL);
    unsigned int hashCode = generateHashCode(item->name);
    if (symbolTable[hashCode] == NULL)
    {
        symbolTable[hashCode] = item;
        if(findStruct(item->name)!=NULL)
        {
            char msg[100] = {0};
            sprintf(msg, "Variable cannot name after a struct name.");
            pError(REDEF_VAR, lineNum, msg);
            return;
        }
    }
    else
    {
        SymbolItem ptr = symbolTable[hashCode];
        while (ptr != NULL)
        {
            if (!strcmp(ptr->name, item->name))
            {
                if (isInStruct<0)
                {
                    char msg[100] = {0};
                    sprintf(msg, "Redefined field. \"%s\".", item->name);
                    pError(REDEF_FIELD, lineNum, msg);
                }
                else
                {
                    char msg[100] = {0};
                    sprintf(msg, "Redefined variable \"%s\".", item->name);
                    pError(REDEF_VAR, lineNum, msg);
                    return;
                }
            }
            ptr = ptr->next;
        }
        ptr = symbolTable[hashCode];
        while (ptr->next != NULL)
            ptr = ptr->next;
        ptr->next = item;
        item->next = NULL;
    }
}

//StmtList : Stmt StmtList
void StmtList(Node *node, Type returnType)
{
    assert(node != NULL);
    if (node->child != NULL)
    {
        Stmt(node->child, returnType);
        if (node->child->next != NULL)
            StmtList(node->child->next, returnType);
    }
}

void Stmt(Node *node, Type returnType)
{
    assert(node != NULL);
    Node *ch = node->child;
    //Stmt : Exp SEMI
    if (!strcmp(ch->name, "Exp"))
    {
        Exp(ch);
    }
    //Stmt : CompSt
    else if (!strcmp(ch->name, "CompSt"))
    {
        CompSt(ch, returnType);
    }
    //Stmt : RETURN Exp SEMI
    else if (!strcmp(ch->name, "RETURN"))
    {
        SymbolItem returnVal = Exp(ch->next);
        if (returnVal == NULL)
            return;
        if (!isSameType(returnVal->type, returnType,false))
        {
            char msg[100] = {0};
            sprintf(msg, "Type mismatched for return. ");
            pError(TYPE_MISMATCHED_RETURN, node->lineNum, msg);
        }
    }
    //Stmt : IF LP Exp RP Stmt
    //Stmt : IF LP Exp RP Stmt ELSE Stmt
    else if (!strcmp(ch->name, "IF"))
    {
        Node *exp = ch->next->next;
        Exp(exp);
        Node *stmt = exp->next->next;
        Stmt(stmt, returnType);
        if (stmt->next != NULL && !strcmp(stmt->next->name, "ELSE"))
            Stmt(stmt->next->next, returnType);
    }
    //Stmt : WHILE LP Exp RP Stmt
    else if (!strcmp(ch->name, "WHILE"))
    {
        Node *exp = ch->next->next;
        Exp(exp);
        Node *stmt = exp->next->next;
        Stmt(stmt, returnType);
    }
}

void addFunc(FuncHead f)
{
    // add it to funcList
    f->prev = funcPtr;
    funcPtr->next = f;
    funcPtr = funcPtr->next;
}

void prepareFunc(FILE* fp)
{
    assert(fp!=NULL);
    //fprintf(fp,"Try to preload read and write\n");
    Type type = (Type)malloc(sizeof(Type_));
    type->kind = BASIC;
    type->info.basicType = B_INT;
    FuncHead readFunc = (FuncHead)malloc(sizeof(FuncHead_));
    readFunc->funcFieldList = NULL;
    readFunc->next = NULL;
    readFunc->prev = NULL;
    readFunc->funcName = newString("read");
    readFunc->returnType = type;
    addFunc(readFunc);

    FuncHead writeFunc = (FuncHead)malloc(sizeof(FuncHead_));
    writeFunc->prev = NULL;
    writeFunc->next = NULL;
    writeFunc->funcName = newString("write");
    writeFunc->returnType = type;
    SymbolItem arg = (SymbolItem)malloc(sizeof(SymbolItem_));
    arg->name = newString("");
    arg->next = NULL;
    arg->type = type;
    arg->reg = NULL;
    // arg->val
    writeFunc->funcFieldList = (FieldList)malloc(sizeof(_FieldList));
    writeFunc->funcFieldList->fieldVar = arg;
    writeFunc->funcFieldList->nextField = NULL;
    addFunc(writeFunc);

    // if(findFunc("read"))
    //     fprintf(fp,"add read successfully\n");
    // if(findFunc("write"))
    //     fprintf(fp,"add write successfully\n");

}