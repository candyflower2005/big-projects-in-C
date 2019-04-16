#ifndef __PARSER_H__
#define __PARSER_H__
#include <stdbool.h>
#include "id_list.h"

const int max;

char * skipGap(char *curr, int *total, char *input, bool need, bool eof);

char * performOperation(char *curr, int *total, char *input, struct idList **list);

#endif
