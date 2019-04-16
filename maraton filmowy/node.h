#ifndef NODE_H
#define NODE_H

typedef struct sNode
{
    struct sNode *prev;
    struct sNode *next;
    int value;
    /* Jeśli użytkownik znajduję się na początku listy children użytkownika ID,
       to headOfList = ID. Jeśli na końcu, to tailOfList = ID. */
    int headOfList;
    int tailOfList;
}node;
extern node *createNode(int);
extern void connectNodes(node*, node*);

#endif
