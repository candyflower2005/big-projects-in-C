#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "id_list.h"

/**
 * Duża liczba ograniczająca maksymalną długość jednego wiersza.
 */
const int max = 500 * 1000 + 419;

/**
 * Przedwczesne zakończenie danych wejściowych.
 */
void eofERROR(){
    fprintf(stderr, "ERROR EOF\n");
    exit(1);
}

/**
 * Błąd składniowy.
 * @param[in] n - numer błędnego znaku
 */
static void syntError(int n){
    fprintf(stderr, "ERROR %d\n", n);
    exit(1);
}

/**
 * Błąd wykonania operacji DEL.
 * @param[in] n - numer pierwszego znaku operatora.
 */
static void delERROR(int n){
    fprintf(stderr, "ERROR DEL %d\n", n);
    exit(1);
}

/**
 * Błąd wykonania operacji NEW.
 * @param[in] n - numer pierwszego znaku operatora.
 */
static void newERROR(int n){
    fprintf(stderr, "ERROR NEW %d\n", n);
    exit(1);
}

/**
 * Błąd wykonania operacji ?.
 * @param[in] n - numer pierwszego znaku operatora.
 */
static void qERROR(int n){
    fprintf(stderr, "ERROR ? %d\n", n);
    exit(1);
}

/**
 * Błąd wykonania operacji >.
 * @param[in] n - numer pierwszego znaku operatora.
 */
static void fERROR(int n){
    fprintf(stderr, "ERROR > %d\n", n);
    exit(1);
}

/**
 * Błąd wykonania operacji @.
 * @param[in] n - numer pierwszego znaku operatora.
 */
static void cERROR(int n){
    fprintf(stderr, "ERROR @ %d\n", n);
    exit(1);
}

/**
 * Sprawdza, czy w danym miejscu znajduje się NEW.
 * @param[in] curr - wskaźnik na ostatni rozpatrywany znak.
 */
static bool detectNEW(char *curr){
    if(*curr == 'N' && *(curr+1) == 'E' && *(curr+2) == 'W') return true;
    return false;
}

/**
 * Sprawdza, czy w danym miejscu znajduje się DEL.
 * @param[in] curr - wskaźnik na ostatni rozpatrywany znak.
 */
static bool detectDEL(char *curr){
    if(*curr == 'D' && *(curr+1) == 'E' && *(curr+2) == 'L') return true;
    return false;
}

/**
 * Sprawdza, czy w danym miejscu znajduje się $$.
 * @param[in] curr - wskaźnik na aktualnie rozpatrywany znak.
 */
static bool detectComment(char *curr){
    return *curr == '$' && *(curr+1) == '$';
}

/**
 * Sprawdza, czy w danym miejscu znajduje się cyfra.
 * @param[in] curr - wskaźnik na ostatni rozpatrywany znak.
 */
static bool detectNumber(char *curr){
    return ('0' <= *curr && *curr <= '9') || *curr == ':' || *curr == ';';
}

/**
 * Sprawdza, czy w danym miejscu znajduje się litera.
 * @param[in] curr - wskaźnik na ostatni rozpatrywany znak.
 */
static bool detectIdentifier(char *curr){
    if('A' <= *curr && *curr <= 'Z') return true;
    if('a' <= *curr && *curr <= 'z') return true;
    return false;
}

/**
 * Sprawdza, czy w danym miejscu znajduje się ?.
 * @param[in] curr - wskaźnik na ostatni rozpatrywany znak.
 */
static bool detectQ(char *curr){
    return *curr == '?';
}

/**
 * Sprawdza, czy w danym miejscu znajduje się >.
 * @param[in] curr - wskaźnik na ostatni rozpatrywany znak.
 */
static bool detectF(char *curr){
    return *curr == '>';
}

/**
 * Sprawdza, czy w danym miejscu znajduje się @.
 * @param[in] curr - wskaźnik na ostatni rozpatrywany znak.
 */
static bool detectA(char *curr){
    return *curr == '@';
}

/** @brief Przeskakuje komentarz.
 * Szuka końca komentarza. Jeśli jest taka potrzeba,
 * wczytuje dane z następnego wiersza wejścia.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total - wskaźnik na numer aktualnego bajta.
 * @param[in] input - zmienna, do której można wczytać następny wiersz wejścia.
 * @param[in] eof	- informacja, czy należy zakończyć działanie programu w wypadku
 * 					  niespodziewanego zakończenia danych wejściowych.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
char * skipComment(char *curr, int *total, char *input, bool eof){
    if(*curr == '\n'){
        *total += 1;
        curr = fgets(input, max, stdin);
        if(curr == NULL){
            if(eof) eofERROR();
            return NULL;
        }
        return skipComment(curr, total, input, eof);
    }
    if(detectComment(curr)){
        *total += 2;
        curr += 2;
        return curr;
    }
    *total += 1;
    curr++;
    return skipComment(curr, total, input, eof);
}

/**
 * Przeskakuje cały numer.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total - wskaźnik na numer aktualnego bajta.
 * @return  Wskaźnik na ostatni rozpatrywany znak.
 */
char * skipNumber(char *curr, int *total){
    if(detectNumber(curr)){
        *total += 1;
        curr++;
        return skipNumber(curr, total);
    }
    return curr;
}

/**
 * Przeskakuje cały identyfikator.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total - wskaźnik na numer aktualnego bajta.
 * @return Wskaźnik na ostatni rozpatrywany znak.
 */
char * skipIdentifier(char *curr, int *total){
    if(detectIdentifier(curr) || detectNumber(curr)){
        *total += 1;
        curr++;
        return skipIdentifier(curr, total);
    }
    return curr;
}

/**
 * Czyta napis.
 * @param[in] start	- pierwszy znak napisu.
 * @param[in] end	- koniec napisu.
 * @return Wartość @p NULL, jeśli nie udało się zaalokować pamięci.
 *		   Wskaźnik na napis w przeciwnym wypadku.
 */
char * readString(char *start, char *end){
    int length = strlen(start) - strlen(end);
    char *newString = malloc(length + 1);
    if(newString == NULL) return NULL;
    newString[length] = '\0';
    for(int i = 0; i < length; i++){
        newString[i] = *(start+i);
    }
    return newString;
}

/** @brief Przeskakuje puste znaki.
 * Przechodzi przez wszystkie znaki, która funkcja isspace() uznaje za puste.
 * Jeśli jest taka potrzeba, wczytuje dane z następnego wiersza.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total - wskaźnik na numer aktualnego bajta.
 * @param[in] input - zmienna, do której można wczytać następny wiersz wejścia.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
char * skipBlank(char *curr, int *total, char *input){
	if(*curr == '\n'){
        *total += 1;
        curr = fgets(input, max, stdin);
        if(curr == NULL) return curr;
        return skipBlank(curr, total, input);
    }
	else if(isspace(*curr) != 0){
        curr++;
        *total += 1;
        return skipBlank(curr, total, input);
    }
    return curr;
}

/** @brief Przeskakuje przez komentarze i puste znaki.
 * Przechodzi przez wszystkie napisy nie będące
 * leksemami (puste znaki oraz komentarze).
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total - wskaźnik na numer aktualnego bajta.
 * @param[in] input - zmienna, do której można wczytać następny wiersz wejścia.
 * @param[in] need	- informacja, czy potrzeba przerwy między poprzednio
 * 					  rozpatrywanym leksemem i następnym.
 * @param[in] eof	- informacja, czy należy zakończyć działanie programu w wypadku
 * 					  niespodziewanego zakończenia danych wejściowych.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
char * skipGap(char *curr, int *total, char *input, bool need, bool eof){
    if(curr == NULL || *curr == '\0') return NULL;
    int start = *total;
    while(isspace(*curr) != 0 || detectComment(curr)){
        if(detectComment(curr)){
            curr += 2;
            *total += 2;
            curr = skipComment(curr, total, input, eof);
            if(curr == NULL) return NULL;
        }
        else{
            curr = skipBlank(curr, total, input);
            if(curr == NULL) return NULL;
        }
    }
    if(need && start == *total){
        syntError(start);
    }
    return curr;
}

/** @brief Wykonuje operację NEW ID.
 * Zaczyna swoje działanie po wykryciu leksemu NEW. Jeśli nie 
 * wystąpił żaden błąd, dodaje nową bazę lub ustawia ją jako aktualną.
 * @param[in] curr    - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total   - wskaźnik na numer aktualnego bajta.
 * @param[in] input   - zmienna, do której można wczytać następny wiersz wejścia.
 * @param[in] listPtr - wskaźnik na listę baz przekierowań.
 * @param[in] start	  - numer pierwszego bajta operacji.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
static char * operationNEWID(char *curr, int *total, char *input, idList **listPtr, int start){
    curr = skipGap(curr, total, input, true, true);
    if(curr == NULL) eofERROR();
    bool forb = detectNEW(curr) || detectDEL(curr);
    int pocz = *total;
    if(!detectIdentifier(curr)) syntError(*total);
    char *newCurr = skipIdentifier(curr, total);
    if(forb && *total - pocz == 3) syntError(pocz);
    char *id = readString(curr, newCurr);
    if(id == NULL) newERROR(start);
    bool ex = exist(*listPtr, id);
    if(!addId(listPtr, id)){
        free(id);
        newERROR(start);
    }
    if(ex) free(id);
    return newCurr;
}

/** @brief Wykonuje operację DEL ID.
 * Zaczyna swoje działanie po wykryciu leksemu DEL i identyfikatora. Jeśli nie
 * wystąpił żaden błąd, usuwa aktualnie rozpatrywaną bazę.
 * @param[in] curr	- wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total	- wskaźnik na numer aktualnego bajta.
 * @param[in] list	- wskaźnik na listę baz przekierowań.
 * @param[in] start	- numer pierwszego bajta operacji.
 * @return Wartość @p NULL, jeśli wystąpił błąd (np. baza nie istnieje).
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
static char * operationDELID(char *curr, int *total, idList **list, int start){
    bool forb = detectNEW(curr) || detectDEL(curr);
    int pocz = *total;
    if(!detectIdentifier(curr)) syntError(pocz);
    char *newCurr = skipIdentifier(curr, total);
    if(forb && *total - pocz == 3) syntError(pocz);
    char *id = readString(curr, newCurr);
    if(id == NULL || !delId(*list, list, id)){
        free(id);
        delERROR(start);
    }
    free(id);
    return newCurr;
}

/** @brief Wykonuje operację DEL NUM.
 * Zaczyna swoje działanie po wykryciu leksemu DEL oraz numeru. Jeśli nie wystąpił
 * żaden błąd, usuwa z aktualnie rozpatrywanej bazy przekierowania o danym prefiksie.
 * @param[in] curr	- wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total	- wskaźnik na numer aktualnego bajta.
 * @param[in] list	- wskaźnik na listę baz przekierowań.
 * @param[in] start	- numer pierwszego bajta operacji.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
static char * operationDELNUM(char *curr, int *total, idList *list, int start){
    char *newCurr = skipNumber(curr, total);
    char *num = readString(curr, newCurr);
    struct PhoneForward *active;
    if(list == NULL || num == NULL || (active=giveCurrent(list)->structure) == NULL){
        free(num);
        delERROR(start);
    }
    phfwdRemove(active, num);
    free(num);
    return newCurr;
}

/** @brief Wykonuje operację ? NUM.
 * Zaczyna swoje działanie po wykryciu leksemu ?. Jeśli nie wystąpił
 * żaden błąd, wypisuje przekierowania na dany numer.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total	- wskaźnik na numer aktualnego bajta.
 * @param[in] input	- zmienna, do której można wczytać następny wiersz wejścia.
 * @param[in] list	- wskaźnik na listę baz przekierowań.
 * @param[in] start	- numer pierwszego bajta operacji.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
static char * operationREVERSE(char *curr, int *total, char *input, idList *list, int start){
    curr = skipGap(curr, total, input, false, true);
    if(curr == NULL) eofERROR();
    if(!detectNumber(curr)) syntError(*total);
    char *newCurr = skipNumber(curr, total);
    char *id = readString(curr, newCurr);
    struct PhoneForward *active;
    if(list == NULL || id == NULL || (active=giveCurrent(list)->structure) == NULL){
        free(id);
        qERROR(start);
    }
    const struct PhoneNumbers *numbers = phfwdReverse(active, id);
    if(numbers == NULL){
        free(id);
        qERROR(start);
    }
    int idx = 0;
    const char *num;
    while ((num = phnumGet(numbers, idx++)) != NULL)
      printf("%s\n", num);
    phnumDelete(numbers);
    free(id);
    free((char*)num);
    return newCurr;
}

/** @brief Wykonuje operację NUM ?.
 * Zaczyna swoje działanie po wykryciu leksemu NUM oraz ?. Jeśli nie wystąpił
 * żaden błąd, wypisuje przekierowanie danego numeru.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total	- wskaźnik na numer aktualnego bajta.
 * @param[in] list	- wskaźnik na listę baz przekierowań.
 * @param[in] num	- numer, którego przekierowanie zostanie zwrócone.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
static char * operationGET(char *curr, int *total, idList *list, char *num){
    idList *currList = giveCurrent(list);
    if(num == NULL || currList == NULL){
        free(num);
        qERROR(*total - 1);
    }
    const struct PhoneNumbers *numbers = phfwdGet(currList->structure, num);
    if(numbers == NULL){
        free(num);
        qERROR(*total - 1);
    }
    printf("%s\n", phnumGet(numbers, 0));
    phnumDelete(numbers);
    free(num);
    return curr;
}

/** @brief Wykonuje operację NUM > NUM.
 * Zaczyna swoje działanie po wykryciu leksemu NUM oraz >. Jeśli nie wystąpił
 * żaden błąd, dodaje przekierowanie numeru.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total	- wskaźnik na numer aktualnego bajta.
 * @param[in] input	- zmienna, do której można wczytać następny wiersz wejścia.
 * @param[in] list	- wskaźnik na listę baz przekierowań.
 * @param[in] num	- numer, który zostanie przekierowany.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
static char * operationADD(char *curr, int *total, char *input, idList *list, char *num){
    int start = *total - 1;
    if(num == NULL) fERROR(start);
    curr = skipGap(curr, total, input, false, false);
    if(curr == NULL){
        free(num);
        eofERROR();
    }
    if(!detectNumber(curr)){
        free(num);
        syntError(*total);
    }
    char *newCurr = skipNumber(curr, total);
    char *num2 = readString(curr, newCurr);
    idList *active = giveCurrent(list);
    if(num2 == NULL || active == NULL || !phfwdAdd(active->structure, num, num2)){
        free(num);
        free(num2);
        fERROR(start);
    }
    free(num);
    free(num2);
    return newCurr;
}

/** @brief Wykonuje operację @ NUM.
 * Zaczyna swoje działanie po wykryciu leksemu @. Jeśli nie wystąpił
 * żaden błąd, wypisuje liczbę nietrywialnych numerów.
 * @param[in] curr  - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total	- wskaźnik na numer aktualnego bajta.
 * @param[in] input	- zmienna, do której można wczytać następny wiersz wejścia.
 * @param[in] list	- wskaźnik na listę baz przekierowań.
 * @param[in] start	- numer pierwszego bajta operacji.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
static char * operationCOUNT(char *curr, int *total, char *input, idList *list, int start){
    curr = skipGap(curr, total, input, false, true);
    if(curr == NULL) eofERROR();
    if(!detectNumber(curr)) syntError(*total);
    char *newCurr = skipNumber(curr, total);
    char *num = readString(curr, newCurr);
    struct PhoneForward *active;
    if(list == NULL || num == NULL || (active=giveCurrent(list)->structure) == NULL){
        free(num);
        cERROR(start);
    }
    int len = strlen(num) - 12;
    if(len < 0) len = 0;
    printf("%zu\n", phfwdNonTrivialCount(active, num, len));
    free(num);
    free((char*)num);
    return newCurr;
}

/** @brief Wykonuje operację.
 * Wykrywa, którą operację należy teraz wykonać. Jeśli dane wejściowe nie
 * pasują do żadnej operacji, kończy działanie programu.
 * @param[in] curr    - wskaźnik na aktualnie rozpatrywany znak.
 * @param[in] total	  - wskaźnik na numer aktualnego bajta.
 * @param[in] input	  - zmienna, do której można wczytać następny wiersz wejścia.
 * @param[in] listPtr - wskaźnik na listę baz przekierowań.
 * @return Wartość @p NULL, jeśli wystąpił błąd.
 *		   Wskaźnik na ostatni rozpatrywany znak w przeciwnym wypadku.
 */
char * performOperation(char *curr, int *total, char *input, idList **listPtr){
    int start = *total;
    idList *list = *listPtr;
    if(detectNEW(curr)){
        curr += 3;
        *total += 3;
        return operationNEWID(curr, total, input, listPtr, start);
    }
    if(detectDEL(curr)){
        curr += 3;
        *total += 3;
        curr = skipGap(curr, total, input, true, true);//DEL12
        if(curr == NULL) eofERROR();
        if(detectNumber(curr)) return operationDELNUM(curr, total, list, start);
        return operationDELID(curr, total, listPtr, start);
    }
    if(detectQ(curr)){
        curr++;
        *total += 1;
        return operationREVERSE(curr, total, input, list, start);
    }
    if(detectNumber(curr)){
        char *endOfNumber = skipNumber(curr, total);
        char *num = readString(curr, endOfNumber);
        char *curr = skipGap(endOfNumber, total, input, false, false);
        if(curr == NULL){
            free(num);
            eofERROR();
        }
        if(detectQ(curr)){
            curr++;
            *total += 1;
            return operationGET(curr, total, list, num);
        }
        if(detectF(curr)){
            curr++;
            *total += 1;
            return operationADD(curr, total, input, list, num);
        }
        else{
            free(num);
            syntError(*total);
        }
    }
    if(detectA(curr)){
        curr++;
        *total += 1;
        return operationCOUNT(curr, total, input, list, start);
    }
    syntError(*total);
    return NULL;
}
