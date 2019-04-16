#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "id_list.h"

/** 
 * Lista, w której trzymane są bazy przekierowań.
 */
idList *list;

/** 
 * Numer aktualnie rozpatrywanego bajta.
 */
int *total;

/** 
 * Napis przechowujący pojedynczy wiersz.
 */
char *input;

/** 
 * Funkcja zwalniająca pamięć.
 */
void del(){
    free(total);
    free(input);
    while(list != NULL) delId(list, &list, list->id);
}

int main()
{
    total = calloc(1, sizeof(int*));
    input = calloc(1, max);
    atexit(del);
    if(total == NULL || input == NULL) return 1;
    *total = 1;
    char *curr = fgets(input, max, stdin);
    curr = skipGap(curr, total, input, false, true);
    while(curr!= NULL){
        curr = performOperation(curr, total, input, &list);
        curr = skipGap(curr, total, input, false, true);
    }
    return 0;
}
