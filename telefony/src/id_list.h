#ifndef __ID_LIST__
#define __ID_LIST__
#include "phone_forward.h"


/**
 * Lista dwukierunkowa przechowująca informacje o bazach przekierowań.
 */
typedef struct idList{
    struct PhoneForward *structure; ///< wskaźnik do bazy przekierowań.
    struct idList *prev;			///< wskaźnik na poprzedni element.
	struct idList *next;			///< wskaźnik na następny element.
    char *id;						///< identyfikator bazy przekierowań.
    bool active;					///< zmienna informująca, czy dana baza jest aktualna.
}idList;

idList * newList();

idList * giveCurrent(idList *list);

idList * exist(idList *list, char *id);

bool addId(idList **headPtr, char *num);

bool delId(idList *list, idList **headPtr, char *id);

#endif
