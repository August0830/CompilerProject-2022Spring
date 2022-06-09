#include "instruction.h"
#define REGCNT 22
#define REGOFFSET 4
#define mipscode(code,args...) fprintf(code,args)
// #define UNLIMITREG
#define SPACE "    "
VarDescriptor varUseInfo;
RegDescriptor regDescriptor[REGCNT];
//0-7 $t0-$t7 $8-$15
//8-15 $s0-$s7 $16-$23
//16-17 $t8-$t9 $24-$25
extern InterCode codes, codePtr;
FILE *instr;
int regcnt;
int lineNo;//for process block update the line number
int fpOffset;
IntStack fpStack;
void initForInst(FILE *ptr)
{
    varUseInfo = NULL;
    for(int i=0;i<REGCNT;++i)
    {
        regDescriptor[i] = (RegDescriptor)malloc(sizeof(RegDescriptor_));
        regDescriptor[i]->varStore = NULL;
        regDescriptor[i]->free = true;
    }
    instr = ptr;
    regcnt=8;
    lineNo==0;
    fpOffset=0;
    fpStack = NULL;
    initCodeFile(ptr);
}

void initCodeFile(FILE* ptr)
{
    mipscode(ptr,".data\n");
    mipscode(ptr,"_prompt: .ascii \"Enter an integer:\"\n");
    mipscode(ptr,"_ret: .asciiz \"\\n\"\n");
    mipscode(ptr,".globl main\n");
    mipscode(ptr,".text\n");
    mipscode(ptr,"read:\n");
    mipscode(ptr,"    li $2, 4\n");
    mipscode(ptr,"    la $a0, _prompt\n");
    mipscode(ptr,"    syscall\n");
    mipscode(ptr,"    li $v0, 5\n");
    mipscode(ptr,"    syscall\n");
    mipscode(ptr,"    jr $ra\n");

    mipscode(ptr,"write:\n");
    mipscode(ptr,"    li $v0, 1\n");
    mipscode(ptr,"    syscall\n");
    mipscode(ptr,"    li $v0, 4\n");
    mipscode(ptr,"    la $a0, _ret\n");
    mipscode(ptr,"    syscall\n");
    mipscode(ptr,"    move $v0, $0\n");
    mipscode(ptr,"    jr $ra\n\n");
}

void pushIntStack(int val)
{
    IntStack tmp = (IntStack)malloc(sizeof(IntStack_));
    tmp->val = val;
    tmp->next = fpStack;
    fpStack = tmp;
}

int popIntStack()
{
    int res = -1;
    if(fpStack!=NULL)
    {
        IntStack tmp = fpStack;
        fpStack = fpStack->next;
        res = tmp->val;
        free(tmp);
    }
    return res;
}

void divideBlock(InterCode code)
{
    int line=0;
    while(!code->isBlockStarter)
    {   
        line++;
        if(code->kind==I_PLUS||
        code->kind==I_SUB||
        code->kind==I_STAR||
        code->kind==I_DIV)
        {
            if(code->u.binop.op1->kind==VARIABLE)
                addLineNoForBlock(code->u.binop.op1->u.var_name,line);
            if(code->u.binop.op2->kind==VARIABLE)
                addLineNoForBlock(code->u.binop.op2->u.var_name,line);
            // if(code->u.binop.res->kind==VARIABLE)
            //     addLineNoForBlock(code->u.binop.res->u.var_name,line);
            // killed
        }
        else if(code->kind==I_ASSIGN)
        {
            // if(code->u.assign.left->kind==VARIABLE)
            //     addLineNoForBlock(code->u.assign.left->u.var_name,line);
            // killed
            if(code->u.assign.right->kind==VARIABLE)
                addLineNoForBlock(code->u.assign.right->u.var_name,line);
        }
        code=code->next;
    }
}

void addLineNoForBlock(char *varName, int lineNo)
{
    VarDescriptor des = searchVarInfoForBlock(varName);
    if (des == NULL)
        des = addVarUseInfoForBlock(varName);
    VarDescriptor ptr = des;
    while (ptr->right != NULL)
    {
        ptr = ptr->right;
    }
    ptr->right = (VarDescriptor)malloc(sizeof(VarDescriptor_));
    ptr->right->left = ptr;
    ptr->right->right=NULL;
    ptr->right->u.lineNo = lineNo;
    ptr->right->fpOffset = -1;
    ptr->right->isAddr = false;
}

VarDescriptor searchVarInfoForBlock(char *varName)
{
    if(varName==NULL)
        return NULL;
    VarDescriptor p = varUseInfo;
    while (p != NULL)
    {
        if (!strcmp(p->u.varName, varName))
        {
            return p;
        }
        p = p->left;
    }
    if(strcmp(varName,""))
        p = addVarUseInfoForBlock(varName);
    return p;
}

VarDescriptor addVarUseInfoForBlock(char *varName)
{
    if (varUseInfo == NULL)
    {
        varUseInfo = (VarDescriptor)malloc(sizeof(VarDescriptor_));
        varUseInfo->left = NULL;
        varUseInfo->right = NULL;
        varUseInfo->u.varName = newString(varName);
        varUseInfo->fpOffset=-1;
        varUseInfo->isAddr=false;
        return varUseInfo;
    }
    else
    {
        VarDescriptor p = (VarDescriptor)malloc(sizeof(VarDescriptor_));
        p->left = NULL;
        p->right = NULL;
        p->fpOffset = -1;
        p->isAddr = false;
        p->u.varName = newString(varName);
        p->left = varUseInfo;
        varUseInfo = p;
        return p;
    }
} //head insert

void translateInstr()
{
    InterCode ptr = codes;
    while(ptr!=NULL)
    {
        if(ptr->isBlockStarter)
        {
            lineNo = 0;
            InterCode tmp = ptr;
            while(tmp!=NULL && tmp->isBlockStarter)
                tmp = tmp->next;
            if(tmp!=NULL)
                divideBlock(tmp);
        }    
        else
        {
            lineNo++;
        }    
        if(isArrBlock(ptr))
        {
            translateInstrArr(ptr);
            ptr=ptr->next;
        }
        else if(ptr->kind == I_ASSIGN || ptr->kind == I_PLUS || ptr->kind == I_SUB
        || ptr->kind == I_STAR || ptr->kind == I_DIV || ptr->kind == I_DEC)
        {
            translateNormalInstr(ptr);
        }    
        else if(ptr->kind==I_LABEL)
        {
            mipscode(instr,"%s:\n",ptr->u.label.labelName);
        }
        else if(ptr->kind==I_JMP)
        {
            mipscode(instr,"    j %s\n",ptr->u.jump.targetLabel);
        }
        else if(ptr->kind==I_COND)
        {
            translateCond(ptr);
        }
        else if(ptr->kind == I_FUNCINFO)
        {
            translateFunc(ptr);
        }
        else if(ptr->kind == I_RETURN)
        {
            translateRet(ptr);
        }
        else if(ptr->kind==I_ARG)
        {
            InterCode args = ptr;
            int argsCnt = 1;
            while(ptr->kind!=I_CALL)
            {
                ptr=ptr->next;
                argsCnt++;
            }    
            
        }
        else if(ptr->kind==I_CALL)
        {
            //no args
        }
        else if(ptr->kind==I_PARAM)
        {
            InterCode param = ptr;
            int cnt=1;
            while(ptr->next->kind!=I_PARAM)
            {
                cnt++;
                ptr=ptr->next;
            }
            translateParam(param,cnt);
        }
        else
        {
            mipscode(instr,"# NORMAL INSTRUCTION %d\n",ptr->kind);
        }  
        //todo
        //process every block; 
        //record var with VarDescriptor by iterating the block
        //give them number while record var info
        ptr = ptr->next;
    }

}

bool isArrBlock(InterCode code)
{
    if((code->kind==I_PLUS || code->kind==I_ASSIGN) && code->u.binop.op1->kind==GETADDR)
    {
        if(code->next->kind==I_ASSIGN && (code->next->u.assign.left->kind==GETADDR || code->next->u.assign.right->kind==GETADDR 
        || code->next->u.assign.left->kind==VALOFADDR || code->next->u.assign.right->kind==VALOFADDR))
            return true;
    }
    return false;
}

void translateParam(InterCode param,int paramCnt)
{
    for(int i=0;i<paramCnt;++i)
    {
        if(i>4)
        {
            VarDescriptor var = searchVarInfoForBlock(param->u.par.varName);
            asssert(var!=NULL);
            var->fpOffset = (i-3)*-4;
        }
        else
        {
            regDescriptor[i]->free=false;
            regDescriptor[i]->varStore = newString(param->u.par.varName);
        }
        param=param->next;
    }
}

void translateCALL(InterCode args,int argsCnt,InterCode call)
{
    //args prepare
    if(args!=NULL && argsCnt!=0)
    {
        int cnt=argsCnt;
        if(cnt>4)
            mipscode(instr,"  addi $sp,$sp,-%d\n",4*(cnt-4));
        while(cnt>4)
        {
            char* reg = ensure(args->u.par.varName,false);
            mipscode(instr,"  sw %s,%d($sp)\n",reg,4*(cnt-5));
            cnt--;
            args = args->next;
        }
        while(cnt>0 && args->kind!=I_CALL)
        {
            char* reg = ensure(args->u.par.varName,false);
            mipscode(instr,"    move $a%d,%s\n",cnt-1);
            cnt--;
            args=args->next;
        }
    }

    //reg
    for(int i=4;i<REGCNT;++i)
    {
        if(!regDescriptor[i]->free && !regDescriptor[i]->varStore!=NULL)
        {
            char* reg = (char*)malloc(sizeof(char)*10);
            sprintf(reg,"$%d",i+REGOFFSET);
            spill(regDescriptor[i],reg);
        }
    }

    pushIntStack(fpOffset);
    fpOffset=0;
    mipscode(instr,"    addi $sp,$sp,-4\n");
    mipscode(instr,"    sw $ra,0($sp)\n");
    mipscode(instr,"    jal %s\n",call->u.callInfo.func);
    mipscode(instr,"    lw $ra,0($sp)\n");
    mipscode(instr,"    addi $sp,$sp,4\n");
    char* reg = ensure(call->u.callInfo.res->u.var_name);
    if(argsCnt>4)
        mipscode(instr,"  addi $sp,$sp,%d\n",4*(argCnt-4));
    fpOffset = popIntStack();
    mipscode(instr,"  move %s,$v0\n",reg);
}

void translateFunc(InterCode code)
{
    mipscode(instr,"%s:\n",code->u.funInfo.funcName);
    mipscode(instr,"    addi $sp,$sp,-4\n");
    mipscode(instr,"    sw $fp 0($sp)\n");
    mipscode(instr,"    move $fp, $sp\n");
}

void translateRet(InterCode code)
{
    //prepare work
    mipscode(instr,"  addi $sp,$sp,%d\n",fpOffset);
    mipscode(instr,"  lw $fp,0($sp)\n");
    fpOffset = popIntStack();
    for(int i=0;i<REGCNT;++i)//including args
    {
        regDescriptor[i]->free=true;
        if(regDescriptor[i]->varStore!=NULL)
            free(regDescriptor[i]->varStore);
        regDescriptor[i]->varStore=NULL;
    }
}

void translateCond(InterCode code)
{
    char* reg1=NULL,*reg2=NULL;
    if(code->u.cond.op1->kind==VARIABLE)
        reg1 = ensure(code->u.cond.op1->u.var_name,false);
    else if(code->u.cond.op1->kind==CONSTANT_I)
    {
        reg1 = ensure("",false);
        mipscode(instr,"    li %s,%d\n",reg1,code->u.cond.op1->u.val_int);
    }
    else if(code->u.cond.op1->kind==CONSTANT_F)
    {
        reg1 = ensure("",false);
        mipscode(instr,"    li %s,%f\n",reg1,code->u.cond.op1->u.val_float);
    }
    else
    {
        printf("error in cond reg1\n");
    }

    if(code->u.cond.op2->kind==VARIABLE)
        reg2 = ensure(code->u.cond.op2->u.var_name,false);
    else if(code->u.cond.op2->kind==CONSTANT_I)
    {
        reg2 = ensure("",false);
        mipscode(instr,"    li %s,%d\n",reg2,code->u.cond.op2->u.val_int);
    }
    else if(code->u.cond.op2->kind==CONSTANT_F)
    {
        reg2 = ensure("",false);
        mipscode(instr,"    li %s,%f\n",reg2,code->u.cond.op2->u.val_float);
    }
    else
    {
        printf("error in cond reg2\n");
    }
    char opInstr[10] = {};
    char* label = newString(code->u.cond.label->u.label.labelName);
    if(!strcmp(code->u.cond.oper,"=="))
        sprintf(opInstr,"beq");
    else if(!!strcmp(code->u.cond.oper,"!="))
        sprintf(opInstr,"bne");
    else if(!!strcmp(code->u.cond.oper,">"))
        sprintf(opInstr,"bgt");
    else if(!!strcmp(code->u.cond.oper,"<"))
        sprintf(opInstr,"blt");
    else if(!!strcmp(code->u.cond.oper,">="))
        sprintf(opInstr,"bge");
    else if(!!strcmp(code->u.cond.oper,"<="))
        sprintf(opInstr,"ble");

    mipscode(instr,"    %s %s,%s,%s\n",opInstr,reg1,reg2,label);
}

void translateInstrArr(InterCode code)
{
    char* res = NULL;
    if(code->kind==I_PLUS)
    {
        char* reg1 = ensure(code->u.binop.op1->u.var_name,false);
        char* reg2 = ensure(code->u.binop.op2->u.var_name,false);
        res = ensure(code->u.binop.res->u.var_name,true);
        VarDescriptor var = searchVarInfoForBlock(code->u.binop.res->u.var_name);
        var->isAddr = true;
        mipscode(instr,"    sub %s,%s,%s\n",res,reg1,reg2);
    }
    else if(code->kind==I_ASSIGN)
    {
        char* reg1 = ensure(code->u.assign.right->u.var_name,false);
        VarDescriptor var = searchVarInfoForBlock(code->u.assign.right->u.var_name);
        var->isAddr = true;
        res = newString(reg1);
    }
    else
        printf("error in arr block 1\n");
    code = code->next;
    if(code->kind!=I_ASSIGN)
        printf("error in arr block 2\n");
    else
    {
        if(code->u.assign.left->kind==VALOFADDR)
        {
            if(!strcmp(code->u.assign.left->u.var_name,code->prev->u.binop.res->u.var_name))
            {
                if(code->u.assign.right->kind == VARIABLE)
                {
                    char* regR = ensure(code->u.assign.right->u.var_name,false);
                    mipscode(instr,"    sw %s,0(%s)\n",regR,res);
                }
                else if(code->u.assign.right->kind==CONSTANT_I)
                {
                    char* regR = ensure("",false);
                    mipscode(instr,"    li %s, %d\n",regR,code->u.assign.right->u.val_int);
                    mipscode(instr,"    sw %s,0(%s)\n",regR,res);
                }
                else if(code->u.assign.right->kind==CONSTANT_F)
                {
                    char* regR = ensure("",false);
                    mipscode(instr,"    li %s, %f\n",regR,code->u.assign.right->u.val_float);
                    mipscode(instr,"    sw %s,0(%s)\n",regR,res);
                }
            }
            else
                printf("error in arr block 3\n");
        }
        else if(code->u.assign.right->kind==VALOFADDR)
        {
            if(!strcmp(code->u.assign.right->u.var_name,code->prev->u.binop.res->u.var_name))
            {
                char* regL = ensure(code->u.assign.left->u.var_name,true);
                mipscode(instr,"    lw %s,0(%s)\n",regL,res);
            }
            else
                printf("error in arr block 4\n");
        }
        else
            printf("error in arr block 5\n");
    }
}

void translateNormalInstr(InterCode code)
{
    if(code->kind == I_ASSIGN)
    {
        if(code->u.assign.right->kind == CONSTANT_F || code->u.assign.right->kind == CONSTANT_I
        || code->u.assign.right->kind == CONSTANT_I)
        {
            if(code->u.assign.left->kind == VARIABLE)
            {
                char* reg = ensure(code->u.assign.left->u.var_name,true);
                if(code->u.assign.right->kind== CONSTANT_F)
                    mipscode(instr,"    li %s, %f\n",reg,code->u.assign.right->u.val_float);
                else if(code->u.assign.right->kind==CONSTANT_I)
                    mipscode(instr,"    li %s, %d\n",reg,code->u.assign.right->u.val_int);
                else if(code->u.assign.right->kind==VARIABLE)
                {
                    char* reg2 = ensure(code->u.assign.right->u.var_name,false);
                    mipscode(instr,"    move %s,%s\n",reg,reg2);
                }  
                else if(code->u.assign.right->kind == VALOFADDR)  
                {
                    mipscode(instr,"#right value is array element\n");
                }
            }
            else if(code->u.assign.left->kind == VALOFADDR)
            {
                mipscode(instr,"#assign to val\n");
            }
            
        }
        else
        {
            char* regLeft = ensure(code->u.assign.left->u.var_name,true);
            char* regRight = ensure(code->u.assign.right->u.var_name,false);
            mipscode(instr,"    move %s, %s\n",regLeft,regRight);
        }
    }
    else if(code->kind == I_PLUS || code->kind == I_SUB)
    {
        if(code->u.binop.op1->kind == VARIABLE && code->u.binop.op2->kind==VARIABLE)
        {
            char op[10]={};
            switch (code->kind)
            {
            case I_PLUS:
                sprintf(op,"add");break;
            case I_SUB:
                sprintf(op,"sub");break;
            default:
                break;
            }
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg1 = ensure(code->u.binop.op1->u.var_name,false);
            char* reg2 = ensure(code->u.binop.op2->u.var_name,false);
            mipscode(instr,"    %s %s, %s, %s\n",op,regRes,reg1,reg2);
        }
        else if(code->u.binop.op1->kind == VARIABLE)
        {
            int sign = 1;
            if(code->kind==I_SUB)
                sign = -1;
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg1 = ensure(code->u.binop.op1->u.var_name,false);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    addi %s,%s,%d\n",regRes,reg1,code->u.binop.op2->u.val_int*sign);
            else if(code->u.binop.op2->kind==CONSTANT_F)
                mipscode(instr,"    addi %s,%s,%f\n",regRes,reg1,code->u.binop.op2->u.val_float*sign);
        }
        else if(code->u.binop.op2->kind == VARIABLE)
        {
            int sign = 1;
            if(code->kind==I_SUB)
                sign = -1;
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg2 = ensure(code->u.binop.op2->u.var_name,false);
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    addi %s,%s,%d\n",regRes,reg2,code->u.binop.op1->u.val_int*sign);
            else if(code->u.binop.op1->kind==CONSTANT_F)
                mipscode(instr,"    addi %s,%s,%f\n",regRes,reg2,code->u.binop.op1->u.val_float*sign);
        }
        else//both constant
        {
            char* reg = ensure(code->u.binop.res->u.var_name,true);
             int sign = 1;
            if(code->kind==I_SUB)
                sign = -1;
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg,code->u.binop.op1->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg,code->u.binop.op1->u.val_float);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    addi %s, %d\n",reg,code->u.binop.op2->u.val_int*sign);
            else
                mipscode(instr,"    addi %s, %f\n",reg,code->u.binop.op2->u.val_float*sign);
        }
    }
    else if(code->kind == I_STAR)
    {
        if(code->u.binop.op1->kind == VARIABLE && code->u.binop.op2->kind==VARIABLE)
        {
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg1 = ensure(code->u.binop.op1->u.var_name,false);
            char* reg2 = ensure(code->u.binop.op2->u.var_name,false);
            mipscode(instr,"    mul %s, %s, %s\n",regRes,reg1,reg2);
        }
        else if(code->u.binop.op1->kind == VARIABLE)
        {
            char* reg = ensure("",false);
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg1 = ensure(code->u.binop.op1->u.var_name,false);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg,code->u.binop.op2->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg,code->u.binop.op2->u.val_float);
            mipscode(instr,"    mul %s,%s,%s\n",regRes,reg1,reg);
        }
        else if(code->u.binop.op2->kind == VARIABLE)
        {
            char* reg = ensure("",false);
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg2 = ensure(code->u.binop.op2->u.var_name,false);
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg,code->u.binop.op1->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg,code->u.binop.op1->u.val_float);
            mipscode(instr,"    mul %s,%s,%s\n",regRes,reg2,reg);
        }   
        else
        {
            char* reg = ensure("",false);
            char* reg1 = ensure("",false);
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg,code->u.binop.op1->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg,code->u.binop.op1->u.val_float);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg1,code->u.binop.op2->u.val_int);
            else
               mipscode(instr,"    li %s, %f\n",reg1,code->u.binop.op2->u.val_float);
            mipscode(instr,"    mul %s,%s,%s\n",regRes,reg,reg1);
        }
    }
    else if(code->kind == I_DIV)
    {
        if(code->u.binop.op1->kind == VARIABLE && code->u.binop.op2->kind==VARIABLE)
        {
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg1 = ensure(code->u.binop.op1->u.var_name,false);
            char* reg2 = ensure(code->u.binop.op2->u.var_name,false);
            mipscode(instr,"    div %s, %s\n",reg1,reg2);
            mipscode(instr,"    mflo %s\n",regRes);
        }
        else if(code->u.binop.op1->kind == VARIABLE)
        {
            char* reg2 = ensure(newString(""),false);
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg1 = ensure(code->u.binop.op1->u.var_name,false);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg2,code->u.binop.op2->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg2,code->u.binop.op2->u.val_float);
            mipscode(instr,"    div %s, %s\n",reg1,reg2);
            mipscode(instr,"    mflo %s\n",regRes);
        }
        else if(code->u.binop.op2->kind == VARIABLE)
        {
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg1 = ensure(newString(""),false);
            char* reg2 = ensure(code->u.binop.op2->u.var_name,false);
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg1,code->u.binop.op1->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg1,code->u.binop.op1->u.val_float);
            mipscode(instr,"    div %s, %s\n",reg1,reg2);
            mipscode(instr,"    mflo %s\n",regRes);
        }   
        else
        {
            char* regRes = ensure(code->u.binop.res->u.var_name,true);
            char* reg = ensure(newString(""),false);
            char* reg1 = ensure(newString(""),false);
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg,code->u.binop.op1->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg,code->u.binop.op1->u.val_float);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg1,code->u.binop.op2->u.val_int);
            else
               mipscode(instr,"    li %s, %f\n",reg1,code->u.binop.op2->u.val_float);
            mipscode(instr,"    div %s,%s\n",reg,reg1);
            mipscode(instr,"    mflo %s\n",regRes);
        }
}
    else if(code->kind == I_DEC)
    {
        mipscode(instr,"    addi $sp, $sp, -%d\n",code->u.arr.size);   
        VarDescriptor var = addVarUseInfoForBlock(code->u.arr.arr->u.var_name);
        var->fpOffset = fpOffset;
        var->isAddr = true;
        fpOffset+=code->u.arr.size;
        //record with VarDescriptor todo
    }
}
char* ensure(char* x,bool isKill)
{
    
    char* reg = (char*)malloc(sizeof(char)*10);
    for(int i=0;i<REGCNT;++i)
    {
        if(!regDescriptor[i]->free && regDescriptor[i]->varStore!=NULL && !strcmp(regDescriptor[i]->varStore,x))
        {
            sprintf(reg,"$%d",i+REGOFFSET);
            return reg;
        }
    }
    #ifdef UNLIMITREG
    sprintf(reg,"$%d",regcnt++);
    #else
    reg = allocate(x);
    #endif
    if(strcmp(x,"")&&!isKill)
    {
        #ifdef UNLIMITREG
        mipscode(instr,"    lw %s, %s\n",reg,x);
        #else
        VarDescriptor var = searchVarInfoForBlock(x);
        assert(var!=NULL);
        if(var->fpOffset!=-1)
        {
            if(var->isAddr)
            {
                if(var->fpOffset==0)
                    mipscode(instr,"    move %s,$fp\n",reg);
                else
                    mipscode(instr,"    sub %s,$fp,%d\n",reg,var->fpOffset);
            }
            else
            {
                mipscode(instr,"    lw %s,%d($fp)\n",reg,-1*var->fpOffset);
                mipscode(instr,"#load %s\n",x);
            }
        }
        #endif
    }
    return reg;
}

char* allocate(char* x)
{
    char* reg = (char*)malloc(sizeof(char)*10);
    for(int i=4;i<REGCNT;++i)
    {
        if (regDescriptor[i]->free)
        {
            regDescriptor[i]->free = false;
            if(strcmp(x,""))
            {
                regDescriptor[i]->varStore = newString(x);
            } //constant can be grabbed  
            else
            {
                free(regDescriptor[i]->varStore);
                regDescriptor[i]->varStore=NULL;
            } 
            sprintf(reg,"$%d",i+REGOFFSET);
            return reg;
        }
    }
    //no free reg
    int nextUser = 0;
    VarDescriptor target = NULL;
    RegDescriptor regDes = NULL;
    int regNum = 0;
    for(int i=4;i<REGCNT;++i)
    {
        if(regDescriptor[i]->varStore==NULL)
        {
            regDescriptor[i]->free=true;
            //clean and move on to next to find
        }    
        else
        {
            VarDescriptor p = searchVarInfoForBlock(regDescriptor[i]->varStore);
            if(p==NULL)
                continue;
            if(p->right!=NULL)
            {
                VarDescriptor tmp = p->right;
                while(tmp!=NULL && tmp->u.lineNo<lineNo)
                {
                    if(tmp->right!=NULL)
                    {
                        tmp->right->left = p;
                        p->right = tmp->right;
                        free(tmp);
                    }
                    else
                    {
                        p->right = NULL;
                        free(tmp);
                    }
                    tmp = p->right;
                }
                //this line: cannot be cover
                if(p->right!=NULL && p->right->u.lineNo>nextUser)
                {
                    target = p;
                    nextUser = p->right->u.lineNo;
                    regDes = regDescriptor[i];
                    //regDes->varStore = NULL;
                    regNum = i;
                }
                else if(p->right==NULL)//will not appear anymore
                {
                    target  = p;
                    regNum = i;
                    regDes = regDescriptor[i];
                    break;
                }
            }
            else
            {
                target  = p;
                regNum = i;
                regDes = regDescriptor[i];
                //regDes->varStore=NULL;
                break;
            }
        }
    }
    sprintf(reg,"$%d",regNum+REGOFFSET);
    spill(regDes,reg);
    if(strcmp(x,""))
        regDes->varStore = newString(x);
    return reg;
}

void spill(RegDescriptor regDes,char* reg)
{
    //write back to mem(stack)
    //user Reg to record the mem addr
    VarDescriptor var = searchVarInfoForBlock(regDes->varStore);
    if(var==NULL)
        return;
    //reg store address no need to store,just cover
    if(!var->isAddr)
    {
        if(var->fpOffset==-1)
        {
            mipscode(instr,"    addi $sp,$sp,-4\n");
            var->fpOffset = fpOffset;
            fpOffset+=4;
        }
        mipscode(instr,"    sw %s,%d($fp)\n",reg,-1*var->fpOffset);   
        mipscode(instr,"#save %s\n",var->u.varName);
    }
    if(regDes->varStore!=NULL)
    {
        free(regDes->varStore);
        regDes->varStore = NULL;
    }
}