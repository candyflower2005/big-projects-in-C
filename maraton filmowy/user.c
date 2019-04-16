#include <stdlib.h>
#include <stdio.h>
#include "user.h"
#include "list.h"
#include "node.h"
#include "pair.h"

user* getUser(int index)
{
    if(userPtr[index] != NULL) return userPtr[index];
    user *newUser = malloc(sizeof(user));
    if(newUser == NULL) exit(1);
    newUser->movies = NULL;
    newUser->children = NULL;
    return userPtr[index] = newUser;
}

static void setHead(int parent, node *child)
{
    userPtr[parent]->children->head = child;
    child->headOfList = parent;
}

static void setTail(int parent, node *child)
{
    userPtr[parent]->children->tail = child;
    child->tailOfList = parent;
}

void addUser(int ParentID, int UserID)
{
    if(userPtr[ParentID] == NULL || userPtr[UserID] != NULL)
    {
        fprintf(stderr, "ERROR\n");
    }
    else
    {
        user *User = getUser(UserID);
        user *Parent = getUser(ParentID);
        if(Parent->children != NULL) Parent->children->tail->tailOfList = -1;
        insertAtTail(&Parent->children, UserID);
        User->NodePtr = Parent->children->tail;
        setTail(ParentID, User->NodePtr);
        if(Parent->children->head == Parent->children->tail)
        {
            User->NodePtr->headOfList = ParentID;
        }
        printf("OK\n");
    }
}

void delUser(int UserID)
{
    if(userPtr[UserID] == NULL || UserID == 0) fprintf(stderr, "ERROR\n");
    else
    {
        user *User = getUser(UserID);
        deleteList(&User->movies);
        node *userNode = User->NodePtr;
        node *Prev = userNode->prev;
        node *Next = userNode->next;
        /* Jeśli User posiada listę potomków oraz sam się w takiej liście znajduje, to poniższy
         * kod "wstawi" listę potomków Usera w jego miejsce. */
        if(User->children != NULL)
        {
            User->children->head->headOfList = -1;
            User->children->tail->tailOfList = -1;
            /* Jeśli User był głową/talią listy potomków swojego ojca, to nową głową/talią
             * tej listy zostanie potomek Usera. */
            if(userNode->headOfList != -1)
            {
                int parent = userNode->headOfList;
                setHead(parent, User->children->head);
            }
            if(userNode->tailOfList != -1)
            {
                int parent = userNode->tailOfList;
                setTail(parent, User->children->tail);
            }
            connectNodes(Prev, User->children->head);
            connectNodes(User->children->tail, Next);
            free(userNode);
        }
        else
        {
            connectNodes(Prev, Next);
            if(userNode->headOfList != -1)
            {
                int parent = userNode->headOfList;
                if(Next != NULL) setHead(parent, Next);
            }
            if(userNode->tailOfList != -1)
            {
                int parent = userNode->tailOfList;
                if(Prev != NULL) setTail(parent, Prev);
            }
            if(Prev == NULL && Next == NULL)
            {
                int parent = userNode->headOfList;
                deleteList(&userPtr[parent]->children);
            }
            else free(userNode);
        }
        freeList(&User->children);
        free(User);
        userPtr[UserID] = NULL;
        printf("OK\n");
    }
}

void addMovie(int UserID, int rating)
{
    if(userPtr[UserID] == NULL || exist(userPtr[UserID]->movies, rating))
    {
        fprintf(stderr, "ERROR\n");
    }
    else
    {
        addElement(&userPtr[UserID]->movies, rating);
        printf("OK\n");
    }
}

void delMovie(int UserID, int rating)
{
    if(userPtr[UserID] == NULL || !exist(userPtr[UserID]->movies, rating))
    {
        fprintf(stderr, "ERROR\n");
    }
    else
    {
        deleteElement(&userPtr[UserID]->movies, rating);
        printf("OK\n");
    }
}

// Funkcja pomocnicza do funkcji marathon, która przechodzi rekurencyjnie całe poddrzewo.
static pair recursion(int currUser, int k, int bestMovie)
{
    user *User = getUser(currUser);
    list *userMovies = User->movies;
    list *result = NULL;
    int resultElements = 0;
    if(userMovies != NULL)
    {
        node *currMovie = userMovies->head;
        if(currMovie->value > bestMovie && k != 0)
        {
            addIncr(&result, &currMovie);
            resultElements++;
        }
        if(result != NULL)
        {
            while(currMovie != NULL && resultElements < k && currMovie->value > bestMovie)
            {
                addIncr(&result, &currMovie);
                resultElements++;
            }
            bestMovie = userMovies->head->value;
        }
    }
	// Rekurencyjne czytanie wyników z poddrzewa użytkownika User.
    if(User->children != NULL)
    {
        node *currChild = User->children->head;
        while(currChild != NULL)
        {
            pair ChildPair = recursion(currChild->value, k, bestMovie);
            list *ChildMovies = ChildPair.List;
            resultElements += ChildPair.n;
            resultElements -= joinLists(&result, &ChildMovies);
            while(resultElements > k)
            {
                deleteLast(&result);
                resultElements--;
            }
            currChild = currChild->next;
        }
    }
    pair resultPair;
    resultPair.List = result;
    resultPair.n = resultElements;
    return resultPair;
}

void marathon(int UserID, int k)
{
    if(userPtr[UserID] == NULL) fprintf(stderr, "ERROR\n");
    else printList(recursion(UserID, k , -1).List);
}
