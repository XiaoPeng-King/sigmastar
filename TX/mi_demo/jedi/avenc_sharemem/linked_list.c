#include <stdio.h>  
#include <stdlib.h>  
#include "linked_list.h"

//create head node
pNODE_S createHeadNode(void)
{
    pNODE_S pHeadNode = (pNODE_S)malloc(sizeof(NODE_S));
    if (NULL == pHeadNode)
    {
        printf("creat head node malloc fail \n");
        return NULL;
    }
    //printf("node: %x \n", pHeadNode);
    pHeadNode->data = 0;
    pHeadNode->next = NULL;

    return pHeadNode;
}

//create a new element and insert to tail node
int insertNewNodeByBack(pNODE_S pHead, int data)
{
    if (pHead == NULL)
    {
        printf("the Head node is error \n");
        return -1;
    }
    
    pNODE_S pTempNode = pHead;
    pNODE_S pNewNode = (pNODE_S)malloc(sizeof(NODE_S));
    
    pNewNode->data = data;
    pNewNode->next = NULL;
    //pNewNode->NodeType = DATA_NODE;
    while (NULL != pTempNode->next)
    {
        //printf("node address : %x \n", pTempNode);
        pTempNode = pTempNode->next;
    }
    pHead->data++;
    //printf("pNew node data : %d \n", pNewNode->data);
    //printf("pNew node type : %d \n", pNewNode->NodeType); 
    pTempNode->next = pNewNode;

    return 0;
}

int findNodeData(pNODE_S pHead, int node)
{
    if (pHead == NULL)
    {
        printf("the Head node is error \n");
        return -1;
    }
    pNODE_S pTemp = pHead;
    int i;
    //printf("pHead : %x \n", pHead);
    //printf("node : %d \n", node);
    if ((node > pHead->data) || (node < 1))
    {
        printf("not find the node, please check node if exist\n");
    }

    for (i=node; i>0; i--)
    {
        //printf("----pTemp : %x \n", pTemp);
        pTemp = pTemp->next;        
    }

    return pTemp->data;
}


int deleteNode(pNODE_S pHead, int node)
{
    if (pHead == NULL)
    {
        printf("the Head node is error \n");
        return -1;
    }

    pNODE_S pTemp = pHead;
    pNODE_S pTempNext = NULL;
    int i;
    if ((node > pHead->data) || (node < 1))
    {
        printf("not find the delete node, please check node if exist\n");
    }

    for (i=node; i>1; i--)
    {
        pTemp = pTemp->next;
    }
    //printf("pTemp address : %x \n", pTemp);
    pTempNext = pTemp->next;
    //printf("pTempNext address : %x \n", pTempNext);
    pTemp->next = pTempNext->next;
    //printf("pTemp next address : %x \n", pTemp->next);
    free(pTempNext);
    pHead->data--;

    return 0;
}

int findAllNodeNumber(pNODE_S pHead)
{
    if (pHead == NULL)
    {
        printf("the Head node is error \n");
        return -1;
    }
    return pHead->data;
}

int cleanAllNode(pNODE_S pHead)
{
    if (pHead == NULL)
    {
        //printf("the Head node is error \n");
        return -1;
    }

    pNODE_S pTemp = pHead, pTempNext = pHead->next;

    while (0 != pHead->data)
    {
        //printf("tempNodeNumber: %d \n", pHead->data);
        //printf("pTempNext : %x \n", pTempNext);
        while (NULL != pTempNext->next)
        {
            //printf("pTemp data : %d \n", pTemp->data);
            pTemp = pTempNext;
            //printf("pTemp: %x \n", pTemp);
            pTempNext = pTemp->next;
            //printf("pTempNext: %x \n", pTempNext);
        }
        free(pTempNext);

        pTempNext = pHead->next;
        //printf("before pTemp->next : %x \n", pTemp->next);
        pTemp->next = NULL;
        //printf("after pTemp->next : %x \n", pTemp->next); 
        //pTemp->next = NULL;
        pHead->data--;
    }

    pHead->next = NULL;
    return 0;
}

int ifSameData(pNODE_S pHead, int data)
{
    pNODE_S pTemp = pHead;
    if (pHead == NULL)
    {
        printf("the Head node is error \n");
        return -1;
    }
    while (NULL != pTemp->next)
    {
        pTemp = pTemp->next;
        //printf("data: %d \n", data);
        //printf("pTemp->data: %d \n", pTemp->data);
        if (data == pTemp->data)
        {
            //printf("node datum are same \n");
            return 1;
        }
    }

    return 0;
}

#if 0
void main(void)
{
    pNODE_S pHead = createHeadNode();
    printf("pHead : %x \n", pHead);
    printf("Node type: %d \n", pHead);
    insertNewNodeByBack(pHead, 1);
    insertNewNodeByBack(pHead, 2);
    insertNewNodeByBack(pHead, 3);
    insertNewNodeByBack(pHead, 4);
    insertNewNodeByBack(pHead, 5);
    insertNewNodeByBack(pHead, 6);
    insertNewNodeByBack(pHead, 7);
    insertNewNodeByBack(pHead, 8);
    insertNewNodeByBack(pHead, 9);
    insertNewNodeByBack(pHead, 10);
    
    //printf("pHead data : %d \n", pHead->data);
    //printf("node : %d , data : %d \n", 1, findNodeData(pHead, 1)); 
    deleteNode(pHead, 10);
    //printf("node : %d , data : %d \n", 1, findNodeData(pHead, 1));
    printf("Node number : %d \n", findAllNodeNumber(pHead));
    //printf("same : %d \n", ifSameData(pHead, 100));
    cleanAllNode(pHead);

}
#endif


