#ifndef USER_H
#define USER_H
#include "list.h"
#include "node.h"
typedef struct sUser
{
    list *movies;
    list *children;
    node *NodePtr; /* Jeśli User jest potomkiem pewnego użytkownika, to NodePtr wskazuje na
                      odpowiadający mu wierzchołek umieszczony w liście dzieci ojca. */
}user;
user *userPtr[65536]; /* Lista wskaźników pozwalająca na dostęp w czasie stałym do użytkownika. */
extern user* getUser(int);
extern void addUser(int, int);
extern void delUser(int);
extern void addMovie(int, int);
extern void delMovie(int, int);
extern void marathon(int, int);

#endif
