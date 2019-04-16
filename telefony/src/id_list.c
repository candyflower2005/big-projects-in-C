#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "phone_forward.h"
#include "id_list.h"

/** @brief Tworzy nową listę.
 * Tworzy nową, pustą listę.
 * @return Wskaźnik na utworzoną strukturę.
 */
idList * newList(){
    idList *list = calloc(1, sizeof(idList));
    return list;
}

/** @brief Zwraca aktualną listę.
 * Przechodzi rekurencyjnie całą listę i szuka aktualnej bazy przekierowań.
 * @param[in] list - wskaźnik na obecny element listy.
 * @return Wartość @p NULL jeśli nie ma aktywnej bazy.
 *		   Wskaźnik na element listy z aktualną bazą w przeciwnym wypadku.
 */
idList * giveCurrent(idList *list){
    if(list == NULL) return NULL;
    if(list->active) return list;
    return giveCurrent(list->next);
}

/** @brief Dodaje bazę do listy.
 * Dodaje bazę o identyfikatorze @p id na początek listy.
 * @param[in] headPtr - podwójny wskaźnik na początek listy.
 * @param[in] id 	  - identyfikator dodawanej bazy.
 * @return Wartość @p false, jeśli nie udało się zaalokować pamięci.
 *		   Wartość @p true w przeciwnym wypadku.
 */
static bool addElement(idList **headPtr, char *id){
    idList *list = *headPtr;
    idList *newEl = newList();
    if(newEl == NULL) return false;
    newEl->structure = phfwdNew();
    newEl->id = id;
    newEl->active = true;
    if(list != NULL){
        newEl->next = list;
        list->prev = newEl;
    }
    *headPtr = newEl;
    return true;
}

/** @brief Sprawdza, czy baza już istnieje.
 * Sprawdza, czy baza o identyfikatorze @p id znajduje się w liście.
 * @param[in] list 	- wskźnik na obecny element listy.
 * @param[in] id 	- identyfikator szukanej bazy.
 * @return Wartość @p NULL, jeśli baza nie istnieje.
 *		   Wskaźnik na bazę w przeciwnym wypadku.
 */
idList * exist(idList *list, char *id){
    if(list == NULL) return NULL;
    if(strcmp(list->id, id) == 0) return list;
    return exist(list->next, id);
}

/** @brief Dodaje bazę.
 * Jeśli baza o identyfikatorze @p id nie istnieje, tworzy
 * nową i dodaje ją na początek listy @p headPtr. Jeśli już
 * istnieje, nic nie robi.
 * @param[in] headPtr - wskźnik na początek listy.
 * @param[in] id	  - identyfikator dodawanej bazy.
 * @return Wartość @p false, jeśli nie udało się zaalokować pamięci.
 *		   Wartość @p true w przeciwnym wypadku.
 */
bool addId(idList **headPtr, char *id){
    idList *list = *headPtr, *curr;
    if((curr=giveCurrent(list)) != NULL) curr->active = false;
    idList *el = exist(list, id);
    if(el != NULL) el->active = true;
    else if(!addElement(headPtr, id)) return false;
    return true;

}

/** @brief Usuwa bazę.
 * Jeśli baza o identyfikatorze @p id istnieje, usuwa ją z listy.
 * @param[in] list 	  - wskaźnik na obecny element listy.
 * @param[in] headPtr - wskaźnik na początek listy.
 * @param[in] id	  - identyfikator usuwanej bazy.
 * @return Wartość @p true, jeśli udało się usunąć bazę.
 *		   Wartość @p false, jeśli takiej bazy nie było.
 */
bool delId(idList *list, idList **headPtr, char *id){
    if(list == NULL) return false;
    if(strcmp(list->id, id) == 0){
        phfwdDelete(list->structure);
        if(list->prev != NULL) list->prev->next = list->next;
        if(list->next != NULL) list->next->prev = list->prev;
        if(list->prev == NULL) *headPtr = list->next;
        free(list->id);
        free(list);
        return true;
    }
    else return delId(list->next, headPtr, id);
}
