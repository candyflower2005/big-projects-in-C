#include<stdlib.h>
#include<stdio.h>
#include "node.h"

node* createNode(int val)
{
    node *newNode = malloc(sizeof(node));
    if(newNode == NULL) exit(1);
    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->value = val;
    newNode->headOfList = -1;
    newNode->tailOfList = -1;
    return newNode;
}

void connectNodes(node *first, node *second)
{
    if(first != NULL) first->next = second;
    if(second != NULL) second->prev = first;
}
