#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"

char input[64];
const char *valid_input[] =
{"addUser","delUser","addMovie","delMovie","marathon"};
const int length_input[] = {7,7,8,8,8};
long result[2];

int choose_operation()
{
    for(int i = 0; i <= 4; i++)
    {
        bool eq = true;
        for(int j = 0; j < length_input[i]; j++)
        {
            if(input[j] != *(valid_input[i]+j)) eq = false;
        }
        if(eq) return i;
    }
    return -1;
}

void getNumbers(int ind)
{
    size_t input_length = strlen(input)-1;
    int s = length_input[ind];
    if(ind == 1)
    {
        char *aux;
        long Number = strtol(input+s+1, &aux, 10);
        unsigned long length = aux-input;
        if(*(input+s) == ' ' && length == input_length) result[0] = Number;
        else
        {
            result[0] = -1;
            result[0] = -1;
        }
    }
    else
    {
        char *aux1, *aux2;
        long firstNumber = strtol(input+s+1, &aux1, 10);
        long secondNumber = strtol(aux1+1, &aux2, 10);
        unsigned long length = aux2-input;
        if(*(input+s) == ' ' && *(aux1) == ' ' && length == input_length)
        {
            result[0] = firstNumber;
            result[1] = secondNumber;
        }
        else
        {
            result[0] = -1;
            result[1] = -1;
        }
    }
}

bool rangeUser(long number)
{
    return 0 <= number && number <= 65535;
}

bool rangeMovie(long number)
{
    return 0 <= number && number <= 2147483647;
}

void clean()
{
    for(int i = 0; i <= 65535; i++)
    {
        if(userPtr[i] != NULL)
        {
            user* User = userPtr[i];
            if(User->children != NULL) deleteList(&User->children);
            if(User->movies != NULL) deleteList(&User->movies);
            free(userPtr[i]);
        }
    }
}

int main()
{
    atexit(clean);
    getUser(0);
    while(fgets(input, sizeof(input), stdin))
    {
        // Wejście jest za długie (przekracza 64 znaki).
        if(strchr(input, '\n') == NULL)
        {
            int aux;
			char first = input[0];
            while((aux = fgetc(stdin)) != '\n' && aux != EOF);
            if(first != '#') fprintf(stderr, "ERROR\n");
        }
        else if(input[0] != '\n' && input[0] != '#')
        switch(choose_operation())
        {
            case 0: //addUser
            {
                getNumbers(0);
                if(!rangeUser(result[0]) || !rangeUser(result[1])) fprintf(stderr, "ERROR\n");
                else addUser(result[0], result[1]);
                break;
            }
            case 1: //delUser
            {
                getNumbers(1);
                if(!rangeUser(result[0])) fprintf(stderr, "ERROR\n");
                else delUser(result[0]);
                break;
            }
            case 2: //addMovie
            {
                getNumbers(2);
                if(!rangeUser(result[0]) || !rangeMovie(result[1])) fprintf(stderr, "ERROR\n");
                else addMovie(result[0], result[1]);
                break;
            }
            case 3: //delMovie
            {
                getNumbers(3);
                if(!rangeUser(result[0]) || !rangeMovie(result[1])) fprintf(stderr, "ERROR\n");
                else delMovie(result[0], result[1]);
                break;
            }
            case 4: //marathon
            {
                getNumbers(4);
                if(!rangeUser(result[0]) || !rangeMovie(result[1])) fprintf(stderr, "ERROR\n");
                else marathon(result[0], result[1]);
                break;
            }
            default: fprintf(stderr, "ERROR\n"); break;
        }
    }
    return 0;
}
