#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#define HEAD_NODE   2
#define DATA_NODE   3

typedef int ElemType; 
//define node type 
typedef struct 
{  
    //char NodeType;
    ElemType data;              //data  
    struct Node *next;          //node point  
}NODE_S,*pNODE_S;

int findNodeData(pNODE_S pHead, int node);
int deleteNode(pNODE_S pHead, int node);
int cleanAllNode(pNODE_S pHead);
pNODE_S createHeadNode(void);
int insertNewNodeByBack(pNODE_S pHead, int data);
int findAllNodeNumber(pNODE_S pHead);
int ifSameData(pNODE_S pHead, int data);



#endif