#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include "common.h"

struct NameValuePair* getPairForName(char* name, struct NameValuePair* pair){
    while(pair != NULL){
        if (strcmp(pair->name, name) == 0){
            return pair;
        }
        pair = pair->next;
    }
    return NULL;
}

char* getValueForName(char* name, struct NameValuePair* pair, char* defaultValue){
 /* Searches the list of name/value pairs for the value that corresponds to the specified name.
    This returns a pointer to the value in the struct, not a copy, so don't change it if this will
    cause problems later. */
    struct NameValuePair* pairForName = getPairForName(name, pair);
    if (pairForName != NULL){
        return pairForName->value;  
    } else {
        return defaultValue;
    }
}

long getValueNumForName(char* name, struct NameValuePair* pair, long defaultValue){
 // Searches the list of name/value pairs for the value that corresponds to the specified name, and converts it to an integer
    char* valueTxt = getValueForName(name, pair, NULL);
    return strToLong(valueTxt, defaultValue);
}
int countChars(char *txt, char c){
    int i;
    for (i=0; txt[i]; txt[i]==c ? i++ : txt++); 
    return i;
}

int* getNumListForName(char* name, struct NameValuePair* pair){
 // Searches the list of name/value pairs for the value that corresponds to the specified name, and converts it to an integer
    char* valueTxt = getValueForName(name, pair, NULL);
    
    if (valueTxt == NULL){
        return NULL;
            
    } else {
        int valCount = countChars(valueTxt, ',') + 1;
        int* result = malloc(sizeof(int) * (valCount + 1));
        
        valueTxt = strdupa(valueTxt);
        char *val = strtok(valueTxt, ",");
        int i = 0, v;
        while (val != NULL) {
            v = strToInt(val, 0);
            if (v>0) {
                result[i] = v;
            } else {
                logMsg(LOG_ERR, "Bad value found by getNumListForName in %s param: %s", name, valueTxt);    
            }
            i++;
            val = strtok(NULL, ",");
        }
        assert(i==valCount);        
        result[valCount] = 0;
        
        return result;
    }
}

struct NameValuePair* makeNameValuePair(char* name, char* value){
 // Allocates and populates a NameValuePair struct
    struct NameValuePair* pair = malloc(sizeof(struct NameValuePair));

    if (name != NULL){
        pair->name = strdup(name);
    } else {
        pair->name = NULL;
    }

    if (value != NULL){
        pair->value = strdup(value);
    } else {
        pair->value = NULL;
    }

    pair->next = NULL;

    return pair;
}

void freeNameValuePairs(struct NameValuePair* param){
 // Free up all the memory used by a NameValuePair struct
    struct NameValuePair* nextParam;
    while (param != NULL){
        nextParam = param->next;
        if (param->name != NULL){
            free(param->name);
        }
        if (param->value != NULL){
            free(param->value);
        }
        free(param);
        param = nextParam;
    }
}

void appendNameValuePair(struct NameValuePair** earlierPair, struct NameValuePair* newPair){
 /* Add the 'newPair' argument to the end of the NameValuePair struct list that begins with
    'earlierPair - this involves stepping through the list one struct at a time, not
    vey efficient for long lists but ok for all the current usage scenarios. */
    if (*earlierPair == NULL){
        *earlierPair = newPair;
    } else {
        struct NameValuePair* curr = *earlierPair;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newPair;
    }
}
