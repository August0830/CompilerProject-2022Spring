#include "intermediate.h"


FILE* code;
int tmpVarCnt;
int varCnt;
int labelCnt;
bool arrRW; // 0 not set -1 read 1 write
bool argsPre;

void prepareIntermediate(FILE* ptr)
{
    assert(ptr!=NULL);
    code = ptr;
    tmpVarCnt = 0;
    varCnt = 0;
    labelCnt =1;
    arrRW = false;
    argsPre = false;
}

void generateInterCode(Node* node)
{
    if(node == NULL)
        return;
    if(!strcmp(node->child->name,"ExtDef"))
    {
        Node* extDef = node->child;
        //ExtDef : Specifier FunDec CompSt
        if(!strcmp(extDef->child->name,"Specifier"))
        {
            Node* funDec = extDef->child->next;
            Node* compSt = funDec->next;
            translate_FunDec(funDec);
            translate_CompSt(compSt);
        }
    }
    else
        generateInterCode(node->child);
    generateInterCode(node->child->next);
}

//FunDec : ID LP VarList RP
void translate_FunDec(Node* funDec)
{
    assert(funDec!=NULL);
    Node* func = funDec->child;
    FuncHead funcInfo = findFunc(func->val.val); 
    //func param
    assert(funcInfo!=NULL);
    fprintf(code,"FUNCTION %s :\n",funcInfo->funcName);
    FieldList args = funcInfo->funcFieldList;
    while(args!=NULL)
    {
        Reg param = newReg(newString(args->fieldVar->name),newVarName());
        if(args->fieldVar->type->kind == ARRAY)
        {
            param->isArr = true;
            param->isAddr = true;
        }       
        fprintf(code,"PARAM %s\n",param->interVarId);
        //args->fieldVar->reg = param;
        copyReg(&(args->fieldVar->reg),&param);
        args = args->nextField;
    }
}

//Exp : ID LP Args RP
//Exp : ID LP RP 
void translate_Exp_Func(Node* Exp, Reg place) 
// place is which return value assign to 
//remember to prepare args
{
    assert(Exp!=NULL);
    Node* funcNode = Exp->child;
    FuncHead funcInfo = findFunc(funcNode->val.val);
    if(funcInfo!=NULL)
    {
        //Exp : ID LP Args RP
        if(!strcmp(funcNode->next->next->name, "Args"))
        {
            Reg argList = newReg(NULL,newVarName());
            if(strcmp(funcInfo->funcName,"write"))
                argsPre = true;
            translate_Args(funcNode->next->next,argList);
            argList =  reverse(argList);
            if(!strcmp(funcInfo->funcName,"write"))
            {
                fprintf(code,"WRITE %s\n",argList->interVarId);
            }
            else
            {
                Reg ptr = argList;
                while(ptr!=NULL)
                {

                    if(ptr->isArr && !ptr->isAddr){
                        fprintf(code,"ARG &%s\n",ptr->interVarId);
                    }                       
                    else
                        fprintf(code,"ARG %s\n",ptr->interVarId);
                    ptr = ptr->next;
                }
                if(place==NULL)
                    place = newReg(NULL,newTempName());
                else if(place->interVarId==NULL)
                    place->interVarId = newTempName();
                fprintf(code,"%s := CALL %s\n",place->interVarId,funcInfo->funcName);
            }
            argsPre = false;
        }
        //Exp : ID LP RP
        else
        {
            if(!strcmp(funcInfo->funcName,"read"))
            {
                fprintf(code,"READ %s\n",place->interVarId);
            }
            else
            {
                assert(place!=NULL);
                fprintf(code, "%s := CALL %s\n",place->interVarId,funcInfo->funcName);
            }
        }
    }
}

//Args : Exp COMMA Args 
//Args : Exp  
void translate_Args(Node* Args,Reg argList)
{
    assert(Args!=NULL);
    Node* Exp = Args->child;
    assert(Exp!=NULL);
    // argList->interVarId = newTempName();
    // argList->codeVarId = newString(Exp->val.val);
    //Exp : Exp LB Exp RB if it is array need to read from array
    bool tmpArrRW = arrRW;
    arrRW=false;
    translate_Exp(Exp,argList);
    arrRW = tmpArrRW;
    SymbolItem item = isArrayExp(Exp);
    if(item!=NULL){
        argList->isArr = true;
    }
    if(Exp->next!=NULL && !strcmp(Exp->next->next->name, "Args"))
    {
        argList->next = newReg(NULL,newVarName());
        translate_Args(Exp->next->next,argList->next);
    }
    
}

Reg reverse(Reg head)
{
    Reg p1 = NULL;
    Reg p2 = NULL;
    if(head==NULL)
        return NULL;
    p1 = head;
    head = head->next;
    p2 = head;
    p1->next=NULL;
    while(head!=NULL)
    {
        head = head->next;
        p2->next = p1;
        p1 = p2;
        p2 = head;
    }
    return p1;
}

//Exp : Exp RELOP Exp 
//Exp : Exp AND Exp
//Exp : Exp OR Exp  
//Exp : NOT Exp
//other
void translate_Cond(Node* Exp,char* labelTrue, char* labelFalse)
{
    assert(Exp!=NULL);
    Node* Exp1 = Exp->child;
    if(!strcmp(Exp1->name,"Exp"))
    {
        Node* Exp2 = Exp1->next->next;
        Node* op = Exp1->next;
        if(!strcmp(op->name,"RELOP"))
        {
            Reg t1 = newReg(NULL,NULL);       
            Reg t2 = newReg(newString(Exp2->val.val),newTempName());
            translate_Exp(Exp1,t1);
            translate_Exp(Exp2,t2);
            char* oper = newString(op->val.val);
            fprintf(code, "IF %s %s %s GOTO %s\n",t1->interVarId,oper,t2->interVarId,labelTrue);
            fprintf(code,"GOTO %s\n",labelFalse);
        }
        else if(!strcmp(op->name,"AND"))
        {
            char* label1 = newLabel();
            translate_Cond(Exp1,label1,labelFalse);
            fprintf(code,"LABEL %s :\n",label1);
            translate_Cond(Exp2,labelTrue,labelFalse);
        }
        else if(!strcmp(op->name,"OR"))
        {
            char* label1 = newLabel();
            translate_Cond(Exp1,labelTrue,label1);
            fprintf(code,"LABEL %s :\n",label1);
            translate_Cond(Exp2,labelTrue,labelFalse);
        }
    }
    //Exp : NOT Exp
    else if(!strcmp(Exp1->name,"NOT"))
    {
        translate_Cond(Exp1->next,labelFalse,labelTrue);
    }
    //other cases
    else
    {
        Reg t1 = newReg(newString(Exp->val.val),newTempName());
        translate_Exp(Exp,t1);
        fprintf(code,"IF %s != #0 GOTO %s\n",t1->interVarId,labelTrue);
        fprintf(code, "GOTO %s\n",labelFalse);
    }
}

void translate_Stmt(Node* Stmt)
{
    assert(Stmt!=NULL);
    Node* child = Stmt->child;
    //Stmt : Exp SEMI 
    if(!strcmp(child->name,"Exp"))
    {
        translate_Exp(child,NULL);
    }
    //Stmt : CompSt
    else if(!strcmp(child->name,"CompSt"))
    {
        translate_CompSt(child);
    }
    //Stmt : RETURN Exp SEMI
    else if(!strcmp(child->name,"RETURN"))
    {
        Node* exp = child->next;
        Reg t1 = newReg(newString(exp->val.val),newTempName());
        // can be optimize
        translate_Exp(exp,t1);
        fprintf(code,"RETURN %s\n",t1->interVarId);
    }
    else if(!strcmp(child->name,"IF"))
    {
        char* label1 = newLabel();
        char* label2 = newLabel();
        Node* exp = child->next->next;
        Node* stmt1 = exp->next->next;
        //Stmt : IF LP Exp RP Stmt
        if(stmt1->next==NULL)
        {
            translate_Cond(exp,label1,label2);
            fprintf(code,"LABEL %s :\n",label1);
            translate_Stmt(stmt1);
            fprintf(code,"LABEL %s :\n",label2);
        }
        //Stmt : IF LP Exp RP Stmt ELSE Stmt
        else if(!strcmp(stmt1->next->name,"ELSE"))
        {
            char* label3 = newLabel();
            Node* stmt2 = stmt1->next->next;
            translate_Cond(exp,label1,label2);
            fprintf(code,"LABEL %s :\n",label1);
            translate_Stmt(stmt1);
            fprintf(code,"GOTO %s\n",label3);
            fprintf(code,"LABEL %s :\n",label2);
            translate_Stmt(stmt2);
            fprintf(code,"LABEL %s :\n",label3);
        }
    }
    //Stmt: WHILE LP Exp RP Stmt
    else if(!strcmp(child->name,"WHILE"))
    {
        char* label1 = newLabel();
        char* label2 = newLabel();
        char* label3 = newLabel();
        Node* exp = child->next->next;
        Node* stmt = exp->next->next;
        fprintf(code,"LABEL %s :\n",label1);
        translate_Cond(exp,label2,label3);
        fprintf(code,"LABEL %s :\n",label2);
        translate_Stmt(stmt);
        fprintf(code,"GOTO %s\n",label1);
        fprintf(code,"LABEL %s :\n",label3);
    }
}

//CompSt : LC DefList StmtList RC
//deflist can be empty!
//CompSt : LC StmtList RC
//StmtList : Stmt StmtList
void translate_CompSt(Node* compSt)
{
    assert(compSt!=NULL);
    Node* defList = NULL;
    Node* stmtList = NULL;
    if(!strcmp(compSt->child->next->name,"DefList"))
    {
        defList = compSt->child->next;
        translate_DefList(defList);
        stmtList = defList->next;
    }
    else
        stmtList = compSt->child->next;
    while(stmtList!=NULL && stmtList->child!=NULL)
    {
        Node* stmt = stmtList->child;
        translate_Stmt(stmt);
        stmtList = stmt->next;
    }
}

//only process Specifier is array
//DefList : Def DefList
//Def : Specifier DecList SEMI
void translate_DefList(Node* defList)
{
    assert(defList!=NULL);
    Node* def = defList->child;
    assert(def!=NULL);
    Node* decList = def->child->next;
    translate_DecList(decList);
    if(def->next!=NULL)
    {
        translate_DefList(def->next);
    }
}

//DecList : Dec
//DecList : Dec COMMA DecList
//Dec : VarDec
//Dec : VarDec ASSIGNOP Exp
//VarDec : VarDec LB INT RB
void translate_DecList(Node* decList)
{
    assert(decList!=NULL);
    Node* dec = decList->child;
    Node* varDec = dec->child;
    assert(varDec!=NULL);
    Reg varDecReg = newReg(NULL,NULL);
    //isArray
    //VarDec : VarDec LB INT RB
    if(!strcmp(varDec->child->name,"VarDec") && varDec->child->next!=NULL && !strcmp(varDec->child->next->name,"LB"))
    {
        
        Node* arrayVarDec = varDec->child;
        translate_VarDec(arrayVarDec,varDecReg);
    }
    //whether vardec is array or not; process assign
    //Dec : VarDec ASSIGNOP Exp
    if(varDec->next!=NULL && !strcmp(varDec->next->name,"ASSIGNOP"))
    {
        Reg var = newReg(NULL,NULL);
        //eg int i=0;
        if(!strcmp(varDec->child->name,"ID"))
        {
            var->codeVarId=newString(varDec->child->val.val);
            var->interVarId = newVarName();
            translate_Exp(varDec->next->next,var);
            SymbolItem varItem = findSymbolInTable(varDec->child->val.val);
            if(varItem!=NULL)
            {
                copyReg(&(varItem->reg),&var);
            }
        }
        //VarDec : VarDec LB INT RB
        //already process above
        else
        {
            //todo assign write array
            Node* exp = varDec->next->next;
            if(isArrayExp(exp)!=NULL)
            {
                SymbolItem item = findSymbolInTable(exp->child->val.val);
                if(item->type->kind==ARRAY)
                {
                    copyArr(varDecReg->codeVarId,item->name);
                }
            }
        }
    }
    if(dec->next!=NULL)
        translate_DecList(dec->next->next);
}

//VarDec : VarDec LB INT RB
//VarDec : ID
void translate_VarDec(Node* varDec, Reg place)
{
    assert(varDec!=NULL);
    Node* child = varDec->child;
    //after id type was recorded
    if(!strcmp(child->name,"ID"))
    {
        SymbolItem array = findSymbolInTable(child->val.val);
        assert(array!=NULL && array->type->kind == ARRAY);
        place->interVarId = newVarName();
        place->codeVarId = newString(child->val.val);
        place->next = NULL;
        place->size = array->type->info.arrayInfo.size;
        copyReg(&(array->reg),&place);
        fprintf(code, "DEC %s %d\n",place->interVarId,place->size);
    }
    else if(!strcmp(child->name,"VarDec"))
    {
        translate_VarDec(child,place);
    }
}

void translate_Exp(Node* Exp, Reg place)
{
    //place can be null
    assert(Exp!=NULL);
    Node* child = Exp->child;
    //Exp : INT
    if(!strcmp(child->name,"INT"))
    {
        int value = child->val.iVal;
        if(place==NULL)
            place = newReg(NULL,newTempName());
        //can be optimize directly assign 
        fprintf(code,"%s := #%d\n",place->interVarId,value);
    }
    //Exp : FLOAT
    else if(!strcmp(child->name,"FLOAT"))
    {
        if(place==NULL)
            place = newReg(NULL,newTempName());
        fprintf(code,"%s := #%s\n",place->interVarId,child->val.val);
    }
    else if(!strcmp(child->name,"Exp"))
    {
        Node* nextOp = child->next;
        //Exp : Exp PLUS Exp
        //Exp : Exp SUB Exp
        //Exp : Exp STAR Exp
        //Exp : Exp DIV Exp
        if(!strcmp(nextOp->name,"PLUS")||!strcmp(nextOp->name,"SUB")||!strcmp(nextOp->name,"STAR")||!strcmp(nextOp->name,"DIV"))
        {
            Reg t1 = newReg(newString(child->val.val),newTempName());
            Node* exp2 = child->next->next;
            Reg t2 = newReg(newString(exp2->val.val),newTempName());
            translate_Exp(child,t1);
            translate_Exp(exp2,t2);
            char op[] = "\0";
            if(!strcmp(nextOp->name,"PLUS"))
                strcpy(op,"+");
            else if(!strcmp(nextOp->name,"SUB"))
                strcpy(op,"-");
            else if(!strcmp(nextOp->name,"STAR"))
                strcpy(op,"*");
            else if(!strcmp(nextOp->name,"DIV"))
                strcpy(op,"/");
            if(place==NULL)
                place = newReg(NULL,newTempName());
            else if(!place->interVarId)
                place->interVarId = newTempName();
            fprintf(code,"%s := %s %s %s\n",place->interVarId,t1->interVarId,op,t2->interVarId);
        }
        //Exp : Exp RELOP Exp 
        //Exp : Exp AND Exp 
        //Exp : Exp OR Exp 
        else if(!strcmp(nextOp->name,"RELOP")||!strcmp(nextOp->name,"AND")||!strcmp(nextOp->name,"OR"))
        {
            char* label1 = newLabel();
            char* label2 = newLabel();
            if(place==NULL)
                place = newReg(NULL,newTempName());
            fprintf(code ,"%s := #0\n",place->interVarId);
            translate_Cond(Exp,label1,label2);
            fprintf(code,"LABEL %s :\n",label1);
            fprintf(code,"%s := #1\n",place->interVarId);
            fprintf(code,"LABEL %s :\n",label2);
        }
        //Exp : Exp LB Exp RB
        else if(!strcmp(nextOp->name,"LB"))
        {
            int* constant = (int*)malloc(sizeof(int));
            *constant = 0;
            Type type = NULL;
            char offset[100]={0};
            Reg arr = calOffset(Exp,offset,&type,constant);
            Reg tmp = newReg(NULL,newTempName());
            if(arr->isAddr)
                fprintf(code,"%s := %s %s\n",tmp->interVarId,arr->interVarId,newString(offset));
            else
                fprintf(code,"%s := &%s + %s\n",tmp->interVarId,arr->interVarId,newString(offset));
            if(*constant!=0)
            {
                fprintf(code,"%s := %s + #%d\n",tmp->interVarId,tmp->interVarId,*constant);
            }
            if(arrRW)//write
                fprintf(code,"*%s := %s\n",tmp->interVarId,place->interVarId);
            else//read
            {
                if(place==NULL)
                    place = newReg(NULL,newTempName());
                else if(place->interVarId==NULL)
                    place->interVarId = newTempName();
                if(argsPre)
                    fprintf(code,"%s := %s\n",place->interVarId,tmp->interVarId);
                else
                    fprintf(code,"%s := *%s\n",place->interVarId,tmp->interVarId);
            }
            // arrRW++;
        }
    }
    else if(!strcmp(child->name,"NOT"))
    {
        char* label1 = newLabel();
        char* label2 = newLabel();
        if(place==NULL)
            place = newReg(NULL,newTempName());
        fprintf(code ,"%s := #0\n",place->interVarId);
        translate_Cond(Exp,label1,label2);
        fprintf(code,"LABEL %s :\n",label1);
        fprintf(code,"%s := #1\n",place->interVarId);
        fprintf(code,"LABEL %s :\n",label2);
    }
    else if(!strcmp(child->name,"SUB"))
    {
        
        Reg t1 = newReg(newString(child->next->val.val),newTempName());
        translate_Exp(child->next,t1);
        if(place==NULL)
            place = newReg(NULL,newTempName());
        fprintf(code,"%s := #0 - %s\n",place->interVarId,t1->interVarId);
    }
    //Exp : ID LP Args RP
    //Exp : ID LP RP
    //Exp : ID
    else if(!strcmp(child->name,"ID"))
    {
        //Exp : ID
        if(child->next == NULL)
        {
            SymbolItem var = findSymbolInTable(child->val.val);
            if(var!=NULL)
            {
                if(var->reg!=NULL)
                {
                    copyReg(&place,&(var->reg));
                }
                else
                {
                    if(place==NULL)
                        place = newReg(newString(child->val.val),newVarName());
                    else if(place->codeVarId==NULL)
                        place->codeVarId = newString(child->val.val);
                    copyReg(&(var->reg),&place);
                }
                //fprintf(code,"%s := %s\n",place->interVarId,child->val.val);
                // can be optimize
            }
        }
        else
        {
            if(place==NULL)
                place = newReg(NULL,newTempName());
            translate_Exp_Func(Exp,place);
        }
    }
    
    
    //Exp : Exp1 ASSIGNOP Exp2
    if(!strcmp(child->name,"Exp") && child->next!=NULL && !strcmp(child->next->name,"ASSIGNOP"))
    {
        Node* exp1 = child;
        
        // Exp1 -> ID
        if(!strcmp(exp1->child->name,"ID"))
        {
            SymbolItem var = findSymbolInTable(exp1->child->val.val);
            assert(var!=NULL);
            //simple var
            if(var->type->kind==T_ID)
            {
                Reg t1 = NULL;
                if(var->reg!=NULL)
                    copyReg(&t1,&(var->reg));
                else
                {
                    t1 = newReg(newString(child->val.val),newVarName());
                    copyReg(&(var->reg),&t1);
                }
                Node* exp2 = child->next->next;
                translate_Exp(exp2,t1);
                if(strcmp(var->reg->interVarId,t1->interVarId))//need to assign
                    fprintf(code,"%s := %s\n",var->reg->interVarId,t1->interVarId);
            }
            //todo array whole assign
            else if(var->type->kind==ARRAY)
            {
                Node* exp2 = child->next->next;
                SymbolItem item = isArrayExp(exp2);
                if(item!=NULL) 
                    copyArr(var->name,item->name);
            }
        }
        //Exp1 : Exp LB Exp RB array assign
        else if(exp1->child->next!=NULL && !strcmp(exp1->child->next->name,"LB"))
        {
            if(place==NULL)
                place = newReg(NULL,newVarName());
            Node* exp2 = exp1->next->next;
            translate_Exp(exp2,place);//get info in place
            arrRW = true;
            translate_Exp(exp1,place);//keep 
            arrRW=false;
        }
    }
    //Exp : LP Exp RP 
    else if(!strcmp(child->name,"LP"))
    {
        translate_Exp(child->next,place);
    }
}

//Exp : Exp LB Exp RB
Reg calOffset(Node* Exp, char* offset, Type* type,int* constant)
{
    Reg id = NULL;
    if(!strcmp(Exp->child->name,"ID"))
    {
        Node* idNode = Exp->child;
        SymbolItem item = findSymbolInTable(idNode->val.val);
        assert(item!=NULL && item->reg!=NULL);
        *type = item->type;
        // id = item->reg;
        copyReg(&id,&(item->reg));
    }
    else
    {
        id = calOffset(Exp->child,offset,type,constant);
        assert((*type)!=NULL && (*type)->kind == ARRAY);
        *type = (*type)->info.arrayInfo.elemType;
    }
    if(Exp->child->next!=NULL)
    {
        Node* indexNode = Exp->child->next->next;

        int size = 4;
        if((*type)->kind==ARRAY)
            size = (*type)->info.arrayInfo.size;
        if(!strcmp(indexNode->name,"Exp"))
        {
            Reg item = newReg(NULL,newTempName());
            translate_Exp(indexNode,item);
            Reg t1 = newReg(NULL,newTempName());
            fprintf(code,"%s := %s * #%d\n",t1->interVarId,item->interVarId,size);
            if(offset!=NULL && strcmp(offset,""))
                fprintf(code,"%s := %s + %s\n",t1->interVarId,t1->interVarId,offset);
            sprintf(offset,"%s",t1->interVarId);
        }       
        else if(!strcmp(indexNode->child->name,"ID"))
        {
            indexNode = indexNode->child;
            SymbolItem item = findSymbolInTable(indexNode->val.val);
            assert(item!=NULL && item->reg!=NULL);
            Reg t1 = newReg(NULL,newTempName());
            
            fprintf(code,"%s := %s * #%d\n",t1->interVarId,item->reg->interVarId,size);
            if(offset!=NULL && strcmp(offset,""))
                fprintf(code,"%s := %s %s\n",t1->interVarId,t1->interVarId,offset);
            sprintf(offset,"+ %s ",t1->interVarId);
        }
        else if(!strcmp(indexNode->child->name,"INT"))
        {
            indexNode = indexNode->child;
            if(indexNode->val.iVal!=0)
            {
                *constant = *constant+indexNode->val.iVal * size;
            }
        }
    }
    return id;
}

SymbolItem isArrayExp(Node* exp)
{
    assert(exp!=NULL);
    Node* child = exp->child;
    if(child->next!=NULL)
        return false;
    else if(strcmp(child->name,"ID"))
        return false;
    SymbolItem item = findSymbolInTable(child->val.val);
    assert(item!=NULL);
    if(item->type->kind == ARRAY)
        return item;
    else
        return NULL;
}

SymbolItem isArrayId(char* id)
{
    SymbolItem item = findSymbolInTable(id);
    assert(item!=NULL);
    if(item->type->kind == ARRAY)
        return item;
    else
        return NULL;
}

char* newTempName()
{
    char tmpVarName[15]={0};
    sprintf(tmpVarName,"t%d",tmpVarCnt);
    tmpVarCnt++;
    return newString(tmpVarName);
}

char* newVarName()
{
    char VarName[15]={0};
    sprintf(VarName,"v%d",varCnt);
    varCnt++;
    return newString(VarName);
}

char* newLabel()
{
    char label[15]={0};
    sprintf(label,"label%d",labelCnt);
    labelCnt++;
    return newString(label);
}

void copyReg(Reg* dst, Reg* src)
{
    if((*dst)==NULL)
    {
        (*dst) = newReg((*src)->codeVarId,(*src)->interVarId);
    }
    else{
        if((*src)->codeVarId!=NULL)
        {
            (*dst)->codeVarId = newString((*src)->codeVarId);
        }
        if((*src)->interVarId!=NULL)
        {
            (*dst)->interVarId = newString((*src)->interVarId);
        }
    }
    (*dst)->size = (*src)->size;
    (*dst)->next = (*src)->next;
    (*dst)->isArr = (*src)->isArr;
    (*dst)->isAddr = (*src)->isAddr;
}

Reg newReg(char* codeVarId,char* interVarId)
{
    Reg t1 = (Reg)malloc(sizeof(_Reg));
    if(codeVarId!=NULL)
        t1->codeVarId = newString(codeVarId);
    else
        t1->codeVarId = NULL;
    if(interVarId!=NULL)
        t1->interVarId = newString(interVarId);
    else
        t1->interVarId = NULL;
    t1->next = NULL;
    t1->size = 0;
    t1->isArr = false;
    t1->isAddr = false;
    return t1;
}

void copyArr(char* dst, char* src)
{
    SymbolItem dstItem = findSymbolInTable(dst);
    SymbolItem srcItem = findSymbolInTable(src);
    assert(dstItem!=NULL && srcItem !=NULL);
    assert(dstItem->type->kind==ARRAY && srcItem->type->kind==ARRAY);
    if(!isSameType(dstItem->type->info.arrayInfo.elemType,srcItem->type->info.arrayInfo.elemType,false))
        return;
    if(dstItem->reg==NULL)
        dstItem->reg = newReg(newString(dstItem->name),newVarName());
    else if(dstItem->reg->interVarId==NULL)
        dstItem->reg->interVarId = newVarName();
    if(srcItem->reg==NULL)
        srcItem->reg = newReg(newString(srcItem->name),newVarName());
    else if(srcItem->reg->interVarId==NULL)
        srcItem->reg->interVarId = newVarName();
    int range = dstItem->type->info.arrayInfo.size;
    if(range>srcItem->type->info.arrayInfo.size)
        range = srcItem->type->info.arrayInfo.size;//count by byte, following same
    
    Reg idx = newReg(NULL,newTempName());
    fprintf(code,"%s := #0\n",idx->interVarId);
    char* label1 = newLabel();
    char* label2 = newLabel();
    char* label3 = newLabel();
    fprintf(code,"LABEL %s :\n",label1);
    fprintf(code,"IF %s < #%d GOTO %s\n",idx->interVarId,range,label2);
    fprintf(code,"GOTO %s\n",label3);
    fprintf(code,"LABEL %s :\n",label2);
    Reg dstPos = newReg(NULL,newTempName());
    Reg srcPos = newReg(NULL,newTempName());
    Reg tmp = newReg(NULL,newTempName());
    fprintf(code,"%s := &%s + %s\n",dstPos->interVarId,dstItem->reg->interVarId,idx->interVarId);
    fprintf(code,"%s := &%s + %s\n",srcPos->interVarId,srcItem->reg->interVarId,idx->interVarId);
    
    fprintf(code,"%s := *%s\n",tmp->interVarId,srcPos->interVarId);
    fprintf(code,"*%s := %s\n",dstPos->interVarId,tmp->interVarId);
    fprintf(code,"%s := %s + #4\n",idx->interVarId,idx->interVarId);
    fprintf(code,"GOTO %s\n",label1);
    fprintf(code,"LABEL %s :\n",label3);
    //unit size can be rewrite but 4 is default for basic elem value
    //higher dimension is treated as one dimension array
}   