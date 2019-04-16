#include "phone_forward.h"
#include "string.h"
#include <stdio.h>

/**
 * Duża liczba ograniczająca maksymalny rozmiar struktury PhoneNmbers.
 */
#define inf 1000*1000*1001

/**
 * Lista przechowująca wszystkie przekierowania do danego wierzchołka.
 */
typedef struct ListOfForwards{
    struct PhoneForward *node;			///< przekierowanie do wierzchołka
    struct ListOfForwards *previous;	///< poprzedni element listy
	struct ListOfForwards *next; 		///< następny element listy
}List;

/**
 * Struktura przechowująca przekierowania numerów.
 * Ma postać drzewa trie, w którym krawędzie utożsamiane są z cyframi,
 * a wierzchołki - z prefiksami utworzonymi przez krawędzie znajdujące
 * się na ścieżce od korzenia do wierzchołka.
 */
typedef struct PhoneForward{
    int depth;                          ///< głębokość wierzchołka
    struct PhoneForward *parent;        ///< ojciec wierzchołka
    struct PhoneForward *nextDigit[12]; ///< synowie wierzchołka
    struct PhoneForward *forwardOut;    ///< przekierowanie danego prefiksu
    List *myPosition;					///< pozycja w liście forwardsIn swojego przekierowania					
    List *forwardsIn;					///< przekierowania innych prefiksów na dany prefiks
}PhoneForward;

/**
 * Struktura przechowująca ciąg numerów telefonów.
 */
typedef struct PhoneNumbers{
    int idx;				///< indeks na ostatni element w tablicy
    char *numbers[inf];		///< tablica zawierająca posortowane rosnąco numery
}PhoneNumbers;

/** @brief Sprawdza, czy napis jest numerem.
 * Sprawdza, czy każde pole @p num jest cyfrą.
 * @param[in] num - wskaźnik na dany napis
 * @return Wartość @p true, jeśli napis jest numerem.
 *		   Wartość @p false w przeciwnym wypadku.
 */
static inline bool isNumber(char const *num){
    if(num == NULL || !*num) return false;
    int length = strlen(num);
    for(int i = 0; i < length; i++){
        if(num[i] == ':' || num[i] == ';') continue;
        if(num[i] < '0' || '9' < num[i]) return false;
    }
    return true;
}

/** @brief Zwraca cyfrę wierzchołka
 * Sprawdza, jaką cyfrę ma wierzchołek @p node.
 * @param[in] node - wskaźnik na dany wierzchołek
 * @return Wartość @p -1, jeśli wierzchołek jest korzeniem.
 *		   Cyfrę wierzchołka w przeciwnym wypadku.
 */
static inline int myDigit(PhoneForward *node){
    if(node->parent == NULL) return -1;
    for(int i = 0; i <= 11; i++){
        if(node->parent->nextDigit[i] == node) return i;
    }
	return -1;
}

static inline int giveDigit(char c){
    if('0' <= c && c <= '9') return c - '0';
    if(c == ':') return 10;
    if(c == ';') return 11;
    return -1;
}

/** @brief Usuwa listę.
 * Usuwa rekurencyjnie listę @p list zawierającą
 * przekierowania.
 * @param[in] list - wskaźnik na listę.
 */
static inline void deleteList(List *list){
    if(list == NULL) return;
    deleteList(list->next);
    list->node->forwardOut = NULL;
    list->node->myPosition = NULL;
    free(list);
}

/** @brief Usuwa przekierowanie.
 * Jeśli istnieje przekierowanie wychodzące z wierzchołka
 * @p node, to usuwa je z listy forwardsIn wierzchołka, na
 * który wskazuje dane przekierowanie.
 * @param[in] node - wskaźnik na wierzchołek.
 */
static void deleteForward(PhoneForward *node){
    if(node->forwardOut == NULL || node->forwardOut == node) return;
    if(node->myPosition->previous == NULL){
        if(node->myPosition->next == NULL){
            node->forwardOut->forwardsIn = NULL;
        }
        else{
            node->forwardOut->forwardsIn = node->myPosition->next;
            node->myPosition->next->previous = NULL;
        }
    }
    else{
        node->myPosition->previous->next = node->myPosition->next;
        if(node->myPosition->next != NULL){
            node->myPosition->next->previous = node->myPosition->previous;
        }
    }
    free(node->myPosition);
}

/** @brief Usuwa przekierowania.
 * Jest to funkcja pomocnicza do funkcji phfwdRemove.
 * Dla danego wierzchołka @p node usuwa rekurencyjnie wszystkie
 * przekierowania z wierzchołków znajdujących się w poddrzewie.
 * @param[in] node - wskaźnik na dany wierzchołek.
 */
static inline void deleteOnlyForwards(PhoneForward *node){
    if(node == NULL) return;
    deleteForward(node);
    node->forwardOut = NULL;
    for(int i = 0; i <= 11; i++){
        deleteOnlyForwards(node->nextDigit[i]);
    }
}

/** @brief Wyznacza ścieżkę w drzewie.
 * Wyznacza ścieżkę zaczynającą się w wierzchołku @p node, której
 * krawędzie są kolejnymi cyframi @p num. Jeśli jest taka potrzeba,
 * tworzy nowe wierzchołki.
 * @param[in] node - wskaźnik na początkowy wierzchołek.
 * @param[in] num  - wskaźnik na numer, który zostanie
 *                   wstawiony do drzewa.
 * @return Wartość NULL, jeśli nie udało się zaalokować pamięci.
 *		   Wskaźnik na końcowy wierzchołek ścieżki w przeciwnym wypadku.
 */
static PhoneForward * giveNode(PhoneForward *node, char const *num){
    if(!*num) return node;
    int digit = giveDigit(*num);
    if(node->nextDigit[digit] == NULL){
        PhoneForward *newNode = phfwdNew();
        if(newNode == NULL) return NULL;
        node->nextDigit[digit] = newNode;
        newNode->parent = node;
        newNode->depth = node->depth + 1;
    }
    return giveNode(node->nextDigit[digit], ++num);
}

/** @brief Wyznacza ścieżkę w drzewie.
 * Wyznacza najdłuższą możliwą ścieżkę zaczynającą się w wierzchołku
 * @p node, której krawędzie są kolejnymi cyframi @p num. Nie tworzy
 * nowych wierzchołków.
 * @param[in] node - wskaźnik na początkowy wierzchołek.
 * @param[in] num  - wskaźnik na numer, który zostanie
 *                   wstawiony do drzewa.
 * @return Wartość NULL, jeśli nie udało się zaalokować pamięci.
 *		   Wskaźnik na końcowy wierzchołek ścieżki w przeciwnym wypadku.
 */
static inline PhoneForward * findNode(PhoneForward *node, char const *num){
    if(*num == '\0') return node;
    int next = giveDigit(*num);
    if(node->nextDigit[next] == NULL) return node;
    return findNode(node->nextDigit[next], ++num);
}

/** @brief Dodaje ścieżkę z drzewa do drzewa.
 * Kopiuje numer wyznaczany przez wierzchołek
 * @p node do drzewa @p tree.
 * @param[in] tree - wskaźnik na drzewo, do którego
 *                   zostanie dodana ścieżka.
 * @param[in] node - wskaźnik na wierzchołek, który
 *                   wskazuje daną ścieżkę.
 * @return Wartość NULL, jeśli nie udało się zaalokować pamięci.
 *		   Wskaźnik na końcowy wierzchołek ścieżki w drzewie w przeciwnym wypadku.
 */
static PhoneForward * addWord(PhoneForward *tree, PhoneForward *node){
    if(node == NULL || myDigit(node) == -1) return tree;
    tree = addWord(tree, node->parent);
    if(tree == NULL) return NULL;
    int digit = myDigit(node);
    if(tree->nextDigit[digit] == NULL){
        PhoneForward *newNode = phfwdNew();
        if(newNode == NULL) return NULL;
        tree->nextDigit[digit] = newNode;
        newNode->parent = tree;
        newNode->depth = tree->depth + 1;
    }
    return tree->nextDigit[digit];
}

/** @brief Dodaje ścieżki z listy do drzewa.
 * Dodaje do drzewa @p tree ścieżki wskazywane przez
 * wierzchołki umieszczone w liście @p list. Do każdej
 * takiej ścieżki dodaje jeszcze sufiks @p num.
 * @param[in] list - wskaźnik na liste zawierającą wierzchołki.
 * @param[in] tree - wskaźnik na drzewo, do którego będą dodawane ścieżki.
 * @param[in] num  - wskaźnik na sufiks, który będzie dodany do każdej ścieżki.
 * @return Wartość @p false, jeśli nie udało się zaalokować pamięci.
 *		   Wartość @p true, jeśli udało się dodać wszystkie ścieżki.
 */
static inline bool iterList(List *list, PhoneForward *tree, char const *num){
    if(list == NULL) return true;
    PhoneForward *node = addWord(tree, list->node);
    if(node == NULL || (node = giveNode(node, num)) == NULL) return false;
    node->forwardOut = node;
    return iterList(list->next, tree, num);
}

/** @brief Wyznacza liczbę przekierowań w drzewie.
 * Wyznacza liczbę wierzchołków w drzewie @p tree,
 * które zawierają przekierowanie.
 * @param[in] tree - wskaźnik na drzewo.
 * @return Zwraca liczbę przekierowań.
 */
static inline int sizeOfTree(PhoneForward *tree){
    if(tree == NULL) return 0;
    int res = 0;
    if(tree->forwardOut != NULL) res++;
    for(int i = 0; i <= 11; i++){
        res += sizeOfTree(tree->nextDigit[i]);
    }
    return res;
}

/** @brief Czyta numery z drzewa.
 * Przeszukuje w głąb całe drzewo. Robi to w kolejności rosnącej -
 * zawsze jako pierwszą wybiera krawędź o mniejszej cyfrze. Podczas
 * przeszukiwania pamięta ścieżkę @p currNum prowadzącą od korzenia
 * do obecnie rozpatrywanego wierzchołka @p node. Jeśli wierzchołek
 * jest końcem pewnego numeru, do wstawia ten numer do struktury @p res.
 * @param[in] node      - wskaźnik na obecnie rozpatrywany wierzchołek.
 * @param[in] res       - wskaźnik na strukturę, do której zapisywane są numery.
 * @param[in] currNum   - wskaźnik na numer wyznaczany przez ścieżkę.
 * @param[in] maxLength - wskaźnik na wielkość zaalokowanej pamięci dla
 *                        numeru wyznaczanego przez ścieżkę.
 */
static void readTree(PhoneForward *node, PhoneNumbers **res, char **currNum, int *maxLength){
    if(node == NULL) return;
    int l = node->depth + 1;
    if(myDigit(node) != -1){ // Dodawanie cyfry na koniec numeru.
        if(*maxLength == l){
            *currNum = realloc(*currNum, 2**maxLength);
            if(currNum == NULL) return;
            *maxLength *= 2;
        }
        (*currNum)[l] = '\0';
        (*currNum)[l - 1] = myDigit(node) + '0';
    }
    else l++;
    if(node->forwardOut != NULL){ // To oznacza, że wierzchołek wyznacza cały numer.
        char *newNumber = calloc(1, *maxLength);
        if(newNumber == NULL){
            phnumDelete(*res);
            *res = NULL;
            return;
        }
        strcpy(newNumber, *currNum);
        (*res)->numbers[(*res)->idx] = newNumber;
        (*res)->idx++;
    }
    for(int i = 0; i <= 11; i++){
        readTree(node->nextDigit[i], res, currNum, maxLength);
        if(*res == NULL) return;
    }
    (*currNum)[l - 1] = '\0'; // Usuwanie cyfry z końca numeru.
}

/** @brief Zwraca strukturę przechowującą ciąg numerów.
 * Zwraca strukturę przechowującą ciąg @p n numerów.
 * @param[in] n - liczba numerów do przechowania.
 * @return Wartość NULL, jeśli nie udało się zaalokować pamięci.
 *		   Wskaźnik na strukturę w przeciwnym wypadku.
 */
static inline PhoneNumbers * phnumNew(int n){
    PhoneNumbers *newPhnum = calloc(1, 2*sizeof(int) + n*sizeof(char*));
    if(newPhnum == NULL) return NULL;
    newPhnum->idx = 0;
    return newPhnum;
}

PhoneForward * phfwdNew(){
    PhoneForward *newNode = calloc(1, sizeof(PhoneForward));
    newNode->depth = -1;
    return newNode;
}

void phfwdDelete(PhoneForward *node){
    if(node != NULL){
        if(node->parent != NULL) node->parent->nextDigit[myDigit(node)] = NULL;
        for(int i = 0; i <= 11; i++) phfwdDelete(node->nextDigit[i]);
        deleteForward(node);
        deleteList(node->forwardsIn);
        free(node);
    }
}

bool phfwdAdd(PhoneForward *node, char const *num1, char const *num2){
    if(!isNumber(num1) || !isNumber(num2)) return false;
    PhoneForward *node1 = giveNode(node, num1);
    PhoneForward *node2 = giveNode(node, num2);
    if(node1 == NULL || node2 == NULL || node1 == node2) return false;

    List *newElement = calloc(1, sizeof(List));
    if(newElement == NULL) return false;
    if(node2->forwardsIn == NULL) node2->forwardsIn = newElement;
    else{
        node2->forwardsIn->previous = newElement;
        newElement->next = node2->forwardsIn;
        node2->forwardsIn = newElement;
    }
    deleteForward(node1);
    node1->forwardOut = node2;
    node2->forwardsIn->node = node1;
    node1->myPosition = node2->forwardsIn;
    return true;
}

void phfwdRemove(PhoneForward *node, char const *num){
    if(!isNumber(num)) return;
    node = findNode(node, num);
	int length = strlen(num);
    if(node->depth + 1 == length) deleteOnlyForwards(node);
}

PhoneNumbers const * phfwdGet(PhoneForward *tree, char const *num){
    PhoneNumbers *res = phnumNew(1);
    if(res == NULL) return NULL;
    if(!isNumber(num)) return res;
    PhoneForward *node = findNode(tree, num);
    while(myDigit(node) != -1 && node->forwardOut == NULL) node = node->parent;
    PhoneForward *sortTree =  phfwdNew(); // Struktura danych, z której będą
                                          // czytane numery w kolejności rosnącej.
    PhoneForward *prefixNode = addWord(sortTree, node->forwardOut);
    if(prefixNode == NULL){
        phfwdDelete(sortTree);
        phnumDelete(res);
        return NULL;
    }
    PhoneForward *suffixNode = giveNode(prefixNode, num + node->depth + 1);
    char *auxChar = calloc(1, 1);
    int *maxLength = calloc(1, sizeof(int));
    if(suffixNode == NULL || auxChar == NULL || maxLength == NULL){
        phfwdDelete(sortTree);
        phnumDelete(res);
        free(auxChar);
        free(maxLength);
        return NULL;
    }
    suffixNode->forwardOut = suffixNode;
    auxChar[0] = '\0';
    *maxLength = 1;

    readTree(sortTree, &res, &auxChar, maxLength);

    free(auxChar);
    free(maxLength);
    phfwdDelete(sortTree);
    return res;
}

PhoneNumbers const * phfwdReverse(PhoneForward *tree, char const *num){
    if(!isNumber(num)){
        PhoneNumbers *res = phnumNew(1);
        if(res == NULL) return NULL;
        return res;
    }
    PhoneForward *sortTree = phfwdNew();
    if(sortTree == NULL) return NULL;
    PhoneForward *node = findNode(tree, num);
    while(myDigit(node) != -1){ // Wstawia do drzewa sortTree przekierowania do wszystkich
								// wierzchołków ze ścieżki od korzenia do wierzchołka
        if(!iterList(node->forwardsIn, sortTree, num + node->depth + 1)) return NULL;
        node = node->parent;
    }
    char *aux = calloc(1, 1);
    int *maxLength = calloc(1, sizeof(int));
    if(aux == NULL || maxLength == NULL){
        phfwdDelete(sortTree);
        free(aux);
        free(maxLength);
        return NULL;
    }
    aux[0] = '\0';
    *maxLength = 1;

    PhoneForward *auxNode = giveNode(sortTree, num);
    if(auxNode == NULL){
        phfwdDelete(sortTree);
        free(aux);
        free(maxLength);
        return NULL;
    }
    auxNode->forwardOut = auxNode;

    PhoneNumbers *res = phnumNew(sizeOfTree(sortTree));
    if(res == NULL){
        phfwdDelete(sortTree);
        free(aux);
        free(maxLength);
        free(res);
        return NULL;
    }
    readTree(sortTree, &res, &aux, maxLength);
    free(aux);
    free(maxLength);
    phfwdDelete(sortTree);
    return res;
}

void phnumDelete(PhoneNumbers const *pnum){
    if(pnum == NULL) return;
    for(int i = 0; i < pnum->idx; i++){
        free(pnum->numbers[i]);
    }
    free((PhoneNumbers*)pnum);
}

char const * phnumGet(PhoneNumbers const *pnum, size_t idx){
    if(pnum == NULL || (int)idx >= pnum->idx) return NULL;
    return pnum->numbers[idx];
}

/** @brief Liczy cyfry w napisie.
 * Przechodzi cały napis i sprawdza, które cyfry w nim wystąpiły.
 * @param[in] arr 	 - tablica, do której informacja jest zapisywana.
 * @param[in] string - dany napis.
 */
static void searchForDigits(bool *arr, char const *string){
    if(*string == '\0') return;
    int idx = giveDigit(*string);
    if(idx != -1) arr[idx] = true;
    searchForDigits(arr, ++string);
}

/** @brief Zwraca liczbę numerów nietrywialnych.
 * Funkcja pomocnicza do funkcji phfwdNonTrivialCount, która przechodzi
 * rekurencyjnie strukturę i zlicza liczbę numerów nietrywialnych.
 * @param[in] node 	- wierzchołek, w którym obecnie się znajduje.
 * @param[in] arr  	- cyfry, z których może się składać numer.
 * @param[in] len	- długość szukanych numerów.
 * @param[in] nmb 	- liczba dostepnych cyfr (liczba wartości true w arr).
 * @return Liczba nietrywialnych numerów.
 */
static size_t count(PhoneForward *node, bool *arr, size_t len, size_t nmb){
    if(len == 0) return 0;
    size_t res = 0;
    if(node->forwardsIn != NULL){
        res = 1;
        while(--len) res *= nmb;
        return res;
    }
    for(int i = 0; i <= 11; i++){
        if(node->nextDigit[i] != NULL && arr[i]){
            res += count(node->nextDigit[i], arr, len - 1, nmb);
        }
    }
    return res;
}

/** @brief Zwraca liczbę numerów nietrywialnych.
 * Funkcja, która liczy liczbę nietrywialnych numerów o
 * długości @p len, których cyfry występują w @p set.
 * @param[in] pf	- struktura przechowująca przekierowania
 * @param[in] set	- napis zawierający dostępne cyfry.
 * @param[in] len	- długość szukanych numerów.
 * @return Liczba nietrywialnych numerów lub 0, gdy wystąpił błąd.
 */
size_t phfwdNonTrivialCount(PhoneForward *pf, char const *set, size_t len){
    if(pf == NULL || set == NULL || strlen(set) == 0) return 0;
    bool *digits = calloc(1, 12 * sizeof(bool));
    if(digits == NULL) return 0;
    searchForDigits(digits, set);
    size_t nmb = 0;
    for(int i = 0; i <= 11; i++){
        if(digits[i]) nmb++;
    }
    if(nmb == 0){
        free(digits);
        return 0;
    }
    size_t res = count(pf, digits, len + 1, nmb);
    free(digits);
    return res;
}
