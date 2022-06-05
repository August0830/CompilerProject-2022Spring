#include "instruction.h"
#define REGCNT 18
#define mipscode(code,args...) fprintf(code,args)
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
    regcnt=0;
    lineNo==-1;
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
        fprintf(instr,"%d\n",ptr->kind);
        //todo
        //process every block; 
        //record var with VarDescriptor by iterating the block
        //give them number while record var info
    }

}

char* ensure(char* x)
{
    #ifdef UNLIMITREG
    char reg[10]={};
    sprintf(reg,"$%d",regcnt++);
    return reg;
    #else
    char* reg = (char*)malloc(sizeof(char)*10);
    for(int i=0;i<REGCNT;++i)
    {
        if(!regDescriptor[i]->free && regDescriptor[i]->varStore!=NULL && !strcmp(regDescriptor[i]->varStore,x))
        {
            return regDescriptor[i]->varStore;
        }
    }
    reg = allocate(x);
    mipscode(instr,"lw %s, %s",reg,x);
    return reg;
    #endif
}

char* allocate(char* x)
{
    char* reg = (char*)malloc(sizeof(char)*10);
    for(int i=0;i<REGCNT;++i)
    {
        if (regDescriptor[i]->free)
        {
            regDescriptor[i]->free = false;
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