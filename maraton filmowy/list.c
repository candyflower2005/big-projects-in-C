#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "list.h"
#include "node.h"

list* createList(int val)
{
    list *newList = malloc(sizeof(list));
    if(newList == NULL) exit(1);
    node *Node = createNode(val);
    newList->head = Node;
    newList->tail = Node;
    return newList;
}

void freeList(list **List)
{
    free(*List);
    *List = NULL;
}

void deleteList(list **ListPtr)
{
    list *List = *ListPtr;
    if(List == NULL) return;
    node *Node = List->head;
    while(Node != NULL)
    {
        node *nextNode = Node->next;
        free(Node);
        Node = nextNode;
    }
    freeList(ListPtr);
}

bool exist(list *List, int val)
{
    if(List == NULL) return false;
    node *currNode = List->head;
    while(currNode != NULL)
    {
        if(currNode->value == val) return true;
        currNode = currNode->next;
    }
    return false;
}

// Funkcja pomocnicza, która przechodzi do następnego wierzchołka.
static void incr(node **Node)
{
    *Node = (*Node)->next;
}

void addIncr(list **ListPtr, node **Node)
{
    insertAtTail(ListPtr, (*Node)->value);
    incr(Node);
}

void insertAtTail(list **ListPtr, int val)
{
    list *List = *ListPtr;
    if(List == NULL) *ListPtr = createList(val);
    else
    {
        node *newNode = createNode(val);
        connectNodes(List->tail, newNode);
        List->tail = newNode;
    }
}

int joinLists(list **firstPtr, list **secondPtr)
{
    list *first = *firstPtr;
    list *second = *secondPtr;
    if(first == NULL)
    {
        *firstPtr = second;
        return 0;
    }
    if(second == NULL) return 0;
    node *currFirst = first->head;
    node *currSecond = second->head;
    list *newList = NULL;
    int same = 0;
    // Wrzucenie do nowej listy największego elementu z obu list.
    if(currFirst->value > currSecond->value) addIncr(&newList, &currFirst);
    else if(currFirst->value < currSecond->value) addIncr(&newList, &currSecond);
    else
    {
        addIncr(&newList, &currFirst);
        incr(&currSecond);
        same++;
    }
    // Wrzucanie malejąco elementów z obu list do nowej listy.
    while(currFirst != NULL || currSecond != NULL)
    {
        if(currFirst == NULL) addIncr(&newList, &currSecond);
        else if(currSecond == NULL) addIncr(&newList, &currFirst);
        else if(currFirst->value < currSecond->value) addIncr(&newList, &currSecond);
        else if(currFirst->value > currSecond->value) addIncr(&newList, &currFirst);
        else
        {
            incr(&currFirst); // Ignorowanie powtarzających się elementów.
            same++;
        }
    }
    deleteList(firstPtr);
    deleteList(secondPtr);
    *firstPtr = newList;
    return same;
}

void addElement(list **ListPtr, int val)
{
    list *List = *ListPtr;
    if(List == NULL) *ListPtr = createList(val);
    else
    {
        list *newList = createList(val);
        joinLists(ListPtr, &newList);
    }
}

void deleteElement(list **ListPtr, int val)
{
    list *List = *ListPtr;
    node *currNode = List->head;
    while(currNode != NULL && currNode->value != val) incr(&currNode);
    node *Prev = currNode->prev;
    node *Next = currNode->next;
    if(currNode == List->head) List->head = Next;
    if(currNode == List->tail) List->tail = Prev;
    connectNodes(Prev, Next);
    free(currNode);
    if(List->head == NULL) freeList(ListPtr);
}

void deleteLast(list **ListPtr)
{
    list *List = *ListPtr;
    node *Last = List->tail;
    node *Prev = Last->prev;
    free(Last);
    List->tail = Prev;
    if(Prev != NULL) Prev->next = NULL;
    else freeList(ListPtr);
}

void printList(list *List)
{
    if(List == NULL) printf("NONE\n");
    else
    {
        node *currNode = List->head;
        while(currNode != NULL)
        {
            printf("%d", currNode->value);
            incr(&currNode);
            if(currNode != NULL) printf(" ");
            else printf("\n");
        }
        deleteList(&List);
    }
}
