#ifndef LIST_H
#define LIST_H
#include "node.h"
#include <stdbool.h>

typedef struct sList
{
    node *head;
    node *tail;
}list;
extern list* createList(int);
extern void freeList(list**);
// Usuwanie listy wraz ze wszystkimi jej elementami:
extern void deleteList(list**);
extern bool exist(list*,int);
/* Funkcja pomocnicza, która wrzuca do listy dany
   element i przechodzi do następnego wierzchołka: */
extern void addIncr(list**,node**);
extern void insertAtTail(list**, int);
/* Funkcja, która z dwóch posortowanych malejąco list tworzy
   jedną oraz zwraca liczbę powtarzających się elementów: */
extern int joinLists(list**, list**);
// Dodaje element do posortowanej malejąco listy:
extern void addElement(list**, int);
extern void deleteElement(list**, int);
extern void deleteLast(list**);
extern void printList(list*);

#endif
