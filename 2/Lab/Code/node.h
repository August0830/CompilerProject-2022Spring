#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "enum.h"

typedef union valType{
    int iVal;
    char* val;
} ValType;

typedef struct node{
    int lineNum; // line number in parentheses
    NodeType type;
    char* name;
    ValType val;

    struct node* child;// node in production
    struct node* next;// next node which is in the same layer
} Node;

typedef Node* ptrNode;

static inline ptrNode newTerminatorNode(NodeType type,int lineNum,char* name,ValType val)
{
    ptrNode token = (ptrNode)malloc(sizeof(Node));

    token->type = type;
    token->lineNum = lineNum;
    token->name = (char*)malloc(sizeof(char)*strlen(name)+1);

    if(type == TOKEN_INT)
    {
        token->val.iVal = val.iVal;
    }
    else
    {
        token->val.val = (char*)malloc(sizeof(char)*strlen(val.val)+1);
        assert(token->val.val!=NULL);
        strncpy(token->val.val,val.val,strlen(val.val));
    }
    
    assert(token->name!=NULL);
    strncpy(token->name,name,strlen(name));

    token->next = NULL;
    token->child = NULL;


    return token;
}

static inline ptrNode newNonterminatorToken(NodeType type,int lineNum,char* name,int argc, ...)
{
    ptrNode token = (ptrNode)malloc(sizeof(Node));
    assert(token!=NULL);

    token->type = type;
    token->lineNum = lineNum;
    
    token->name = (char*)malloc(sizeof(char)*strlen(name));
    strncpy(token->name,name,strlen(name));
    //fill basic info

    va_list valist;
    va_start(valist,argc);
    
    //fill child and its next with tokens in production
    ptrNode tmp = va_arg(valist,ptrNode);
    token->child = tmp;
    for(int i=1;i<argc;++i)
    {
        tmp->next = va_arg(valist,ptrNode);
        if(tmp->next!=NULL)
        {
            tmp = tmp->next;
        }
    }
    va_end(valist);

    return token;
}

static inline void deleteNode(ptrNode node)
{
    if(node == NULL)
        return;
    while(node->child!=NULL)
    {
        ptrNode tmp = node->child;
        node->child = node->child->next;
        deleteNode(tmp);
    }
    free(node->name);
    if(node->type != TOKEN_INT){
        free(node->val.val);
        node->val.val = NULL;
    }
    
    free(node);
    node->name = NULL;
    
    node = NULL;
}

static inline void printNodeTree(ptrNode node,int depth)
{
    if(node == NULL)
        return;
    for(int i=0;i<depth;++i)
    {
        printf("  ");
    }
    printf("%s",node->name);

    if(node->type == NOT_TOKEN)
    {
        printf(" (%d)",node->lineNum);
    }
    else if(node->type == TOKEN_TYPE 
    || node->type == TOKEN_ID)
    {
        printf(": %s",node->val.val);
    }
    else if(node->type == TOKEN_INT)
    {
        printf(": %d",node->val.iVal);
    }
    else if(node->type == TOKEN_FLOAT)
    {
        printf(": %lf",atof(node->val.val));
    }
    printf("\n");
    printNodeTree(node->child,depth+1);
    printNodeTree(node->next,depth);
}