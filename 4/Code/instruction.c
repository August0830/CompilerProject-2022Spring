#include "instruction.h"
#define REGCNT 18
#define mipscode(code,args...) fprintf(code,args)
#define UNLIMITREG
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
    lineNo==-1;
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

void addLineNo(char *varName, int lineNo)
{
    VarDescriptor des = searchVarInfo(varName);
    if (des == NULL)
        des = addVarUseInfo(varName);
    VarDescriptor ptr = des;
    while (ptr->right != NULL)
    {
        ptr = ptr->right;
    }
    ptr->right = (VarDescriptor)malloc(sizeof(VarDescriptor_));
    ptr->right->left = ptr;
    ptr->right->u.lineNo = lineNo;
}

VarDescriptor searchVarInfo(char *varName)
{
    VarDescriptor p = varUseInfo;
    while (p != NULL)
    {
        if (!strcmp(p->u.varName, varName))
        {
            return p;
        }
        p = p->left;
    }
    return p;
}

VarDescriptor addVarUseInfo(char *varName)
{
    if (varUseInfo == NULL)
    {
        varUseInfo = (VarDescriptor)malloc(sizeof(VarDescriptor_));
        varUseInfo->left = NULL;
        varUseInfo->right = NULL;
        varUseInfo->u.varName = newString(varName);
        return varUseInfo;
    }
    else
    {
        VarDescriptor p = (VarDescriptor)malloc(sizeof(VarDescriptor_));
        p->left = NULL;
        p->right = NULL;
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
        if(code->next->kind==I_ASSIGN && (code->next->u.assign.left->kind==GETADDR || code->next->u.assign.right->kind==GETADDR))
            return true;
    }
    return false;
}

void translateInstrArr(InterCode code)
{
    char* res = NULL;
    if(code->kind==I_PLUS)
    {
        char* reg1 = ensure(code->u.binop.op1->u.var_name);
        char* reg2 = ensure(code->u.binop.op2->u.var_name);
        res = ensure(code->u.binop.res->u.var_name);
        mipscode(instr,"    sub %s,%s,%s\n",res,reg1,reg2);
    }
    else if(code->kind==I_ASSIGN)
    {
        res = newString(reg1);
    }
    else
        printf("error in arr block 1\n");
    code = code->next;
    if(code->kind!=I_ASSIGN)
        printf("error in arr block 2\n");
    else
    {
        if(code->u.assign.left->kind==GETADDR)
        {
            if(!strcmp(code->u.assign.left->u.var_name,code->prev->u.binop.res->u.var_name))
            {
                char* regR = ensure(code->u.assign.right->u.var_name);
                mipscode(instr,"    sw %s,0(%s)\n",regR,res);
            }
            else
                printf("error in arr block 3\n");
        }
        else if(code->u.assign.right->kind==GETADDR)
        {
            if(!strcmp(code->u.assign.right->u.var_name,code->prev->u.binop.res->u.var_name))
            {
                char* regL = ensure(code->u.assign.left->u.var_name);
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
                char* reg = ensure(code->u.assign.left->u.var_name);
                if(code->u.assign.right->kind== CONSTANT_F)
                    mipscode(instr,"    li %s, %f\n",reg,code->u.assign.right->u.val_float);
                else if(code->u.assign.right->kind==CONSTANT_I)
                    mipscode(instr,"    li %s, %d\n",reg,code->u.assign.right->u.val_int);
                else if(code->u.assign.right->kind==VARIABLE)
                {
                    char* reg2 = ensure(code->u.assign.right->u.var_name);
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
            char* regLeft = ensure(code->u.assign.left->u.var_name);
            char* regRight = ensure(code->u.assign.right->u.var_name);
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
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg1 = ensure(code->u.binop.op1->u.var_name);
            char* reg2 = ensure(code->u.binop.op2->u.var_name);
            mipscode(instr,"    %s %s, %s, %s\n",op,regRes,reg1,reg2);
        }
        else if(code->u.binop.op1->kind == VARIABLE)
        {
            int sign = 1;
            if(code->kind==I_SUB)
                sign = -1;
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg1 = ensure(code->u.binop.op1->u.var_name);
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
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg2 = ensure(code->u.binop.op2->u.var_name);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    addi %s,%s,%d\n",regRes,reg2,code->u.binop.op1->u.val_int*sign);
            else if(code->u.binop.op2->kind==CONSTANT_F)
                mipscode(instr,"    addi %s,%s,%f\n",regRes,reg2,code->u.binop.op1->u.val_float*sign);
        }
        else//both constant
        {
            char* reg = ensure(newString(""));
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
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg1 = ensure(code->u.binop.op1->u.var_name);
            char* reg2 = ensure(code->u.binop.op2->u.var_name);
            mipscode(instr,"    mul %s, %s, %s\n",regRes,reg1,reg2);
        }
        else if(code->u.binop.op1->kind == VARIABLE)
        {
            char* reg = ensure(newString(""));
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg1 = ensure(code->u.binop.op1->u.var_name);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg,code->u.binop.op2->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg,code->u.binop.op2->u.val_float);
            mipscode(instr,"mul %s,%s,%s\n",regRes,reg1,reg);
        }
        else if(code->u.binop.op2->kind == VARIABLE)
        {
            char* reg = ensure(newString(""));
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg2 = ensure(code->u.binop.op2->u.var_name);
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg,code->u.binop.op1->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg,code->u.binop.op1->u.val_float);
            mipscode(instr,"    mul %s,%s,%s\n",regRes,reg2,reg);
        }   
        else
        {
            char* reg = ensure(newString(""));
            char* reg1 = ensure(newString(""));
            char* regRes = ensure(code->u.binop.res->u.var_name);
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
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg1 = ensure(code->u.binop.op1->u.var_name);
            char* reg2 = ensure(code->u.binop.op2->u.var_name);
            mipscode(instr,"    div %s, %s\n",reg1,reg2);
            mipscode(instr,"    mflo %s\n",regRes);
        }
        else if(code->u.binop.op1->kind == VARIABLE)
        {
            char* reg1 = ensure(newString(""));
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg2 = ensure(code->u.binop.op1->u.var_name);
            if(code->u.binop.op2->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg1,code->u.binop.op2->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg1,code->u.binop.op2->u.val_float);
            mipscode(instr,"    div %s, %s\n",reg1,reg2);
            mipscode(instr,"    mflo %s\n",regRes);
        }
        else if(code->u.binop.op2->kind == VARIABLE)
        {
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg1 = ensure(newString(""));
            char* reg2 = ensure(code->u.binop.op2->u.var_name);
            if(code->u.binop.op1->kind==CONSTANT_I)
                mipscode(instr,"    li %s, %d\n",reg1,code->u.binop.op1->u.val_int);
            else
                mipscode(instr,"    li %s, %f\n",reg1,code->u.binop.op1->u.val_float);
            mipscode(instr,"    div %s, %s\n",reg1,reg2);
            mipscode(instr,"    mflo %s\n",regRes);
        }   
        else
        {
            char* regRes = ensure(code->u.binop.res->u.var_name);
            char* reg = ensure(newString(""));
            char* reg1 = ensure(newString(""));
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
        //record with VarDescriptor todo
    }
}
char* ensure(char* x)
{
    
    char* reg = (char*)malloc(sizeof(char)*10);
    for(int i=0;i<REGCNT;++i)
    {
        if(!regDescriptor[i]->free && regDescriptor[i]->varStore!=NULL && !strcmp(regDescriptor[i]->varStore,x))
        {
            sprintf(reg,"$%d",i+8);
            return reg;
        }
    }
    #ifdef UNLIMITREG
    sprintf(reg,"$%d",regcnt++);
    #else
    reg = allocate(x);
    #endif
    if(strcmp(x,""))
        mipscode(instr,"    lw %s, %s\n",reg,x);
    return reg;
}

char* allocate(char* x)
{
    char* reg = (char*)malloc(sizeof(char)*10);
    for(int i=0;i<REGCNT;++i)
    {
        if (regDescriptor[i]->free)
        {
            regDescriptor[i]->free = false;
            if(strcmp(x,""))
                regDescriptor[i]->varStore = newString(x);
            sprintf(reg,"$%d",i+8);
            return reg;
        }
    }
    int nextUser = 0;
    VarDescriptor target = NULL;
    RegDescriptor regDes = NULL;
    for(int i=0;i<REGCNT;++i)
    {
        VarDescriptor p = searchVarInfo(regDescriptor[i]->varStore);
        if(p!=NULL)
        {
            if(p->right->u.lineNo == lineNo)
            {
                if(p->right->right!=NULL)
                {
                    VarDescriptor tmp = p->right;
                    p->right->right->left = p;
                    p->right = tmp->right;
                    free(tmp);
                }//delete the record at this line
            }
            if(p->right->u.lineNo>nextUser)
            {
                target = p;
                nextUser = p->right->u.lineNo;
                regDes = regDescriptor[i];
            }
        }
    }
    spill(regDes);
}

void spill(RegDescriptor regDes)
{
    //write back to mem(stack)
    //user Reg to record the mem addr
}