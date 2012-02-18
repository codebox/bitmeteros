/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include "bmws.h"
#include "common.h"

/*
Contains code for creating and processing Request structs.
*/

static void logRequest(struct Request* request);

char* getValueForName(char* name, struct NameValuePair* pair, char* defaultValue){
 /* Searches the list of name/value pairs for the value that corresponds to the specified name.
    This returns a pointer to the value in the struct, not a copy, so don't change it if this will
    cause problems later. */
    while(pair != NULL){
        if (strcmp(pair->name, name) == 0){
            return pair->value;
        }
        pair = pair->next;
    }
    return defaultValue;
}

long getValueNumForName(char* name, struct NameValuePair* pair, long defaultValue){
 // Searches the list of name/value pairs for the value that corresponds to the specified name, and converts it to an integer
    char* valueTxt = getValueForName(name, pair, NULL);
    return strToLong(valueTxt, defaultValue);
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

static struct NameValuePair escapeChars[] = {
    {"%27", "'", NULL},
    {"%20", " ", NULL},
    {"%3C", "<", NULL},
    {"%3E", ">", NULL},
    {NULL, NULL, NULL}
};
static char* unescapeValue(char* value){
    struct NameValuePair thisEscape;
    char from[BUFSIZE], to[BUFSIZE]; 
    strncpy(from, value, BUFSIZE);
    
    int escapeIndex = 0;
    
    while ((thisEscape = escapeChars[escapeIndex++]).name != NULL) {
        char* nextFromReadPosn = from;
        char* nextToWritePosn = to;
        char* match;
        int lenOfPartBeforeMatch, lenOfReplacedTxt, lenOfReplacementTxt;
        
        while ((match = strstr(nextFromReadPosn, thisEscape.name)) != NULL){
            lenOfPartBeforeMatch = match - nextFromReadPosn;
            strncpy(nextToWritePosn, nextFromReadPosn, lenOfPartBeforeMatch);
            
            lenOfReplacedTxt    = strlen(thisEscape.name);
            lenOfReplacementTxt = strlen(thisEscape.value);
            
            nextToWritePosn  += lenOfPartBeforeMatch;
            nextFromReadPosn += lenOfPartBeforeMatch;
            strncpy(nextToWritePosn, thisEscape.value, lenOfReplacementTxt);
            
            nextToWritePosn  += lenOfReplacementTxt;
            nextFromReadPosn += lenOfReplacedTxt;
        }
        strcpy(nextToWritePosn, nextFromReadPosn);
        strncpy(from, to, BUFSIZE);
    }
    return strdup(to);    
}

struct Request* parseRequest(char* requestTxt){
 // Allocates a new Request struct and populates it with the contents of the request
    char httpMethod[BUFSIZE];
    char httpResource[BUFSIZE];
    char httpHeader[BUFSIZE];

    if (isLogDebug()){
        logMsg(LOG_DEBUG, "Request: %s", requestTxt);
    }

 // Extract the HTTP method and resource
    sscanf(requestTxt, "%s %s HTTP%*4c %2056c", httpMethod, httpResource, httpHeader);
    struct Request* request = malloc(sizeof(struct Request));
    char* path;
    char* paramPair;
    char* paramName;
    char* paramValue;
    char* paramEquals;
    char* httpResourceCopy = strdupa(httpResource);
    int paramNameLen;

    request->method  = strdup(httpMethod);
    request->params  = NULL;
    request->headers = NULL;

 /* We parse the httpResource value to extract the parameters, if any. Do this on a copy because
    strtok alters the contents of the string. */
    path = strtok(httpResourceCopy, "?");
    if (path == NULL){
     // No parameters
        request->path = strdup(httpResourceCopy);
    } else {
        request->path = strdup(path);

     // Split the path into 'name=value' parts
        while(1){
            paramPair = strtok(NULL, "&");

            if (paramPair == NULL){
             // Reached the end of the parameter list
                break;

            } else {
                paramEquals = strchr(paramPair, '=');
                if (paramEquals == NULL){
                 // There was no '=' sign in this part, so ignore it
                    continue;
                } else {
                 // Get the part before the '=' character
                    paramNameLen = paramEquals - paramPair;
                    paramName = malloc(paramNameLen + 1);
                    strncpy(paramName, paramPair, paramNameLen + 1);
                    paramName[paramNameLen] = 0;

                    if (strcmp(paramEquals, "=") == 0){
                     // No value was supplied - this is valid in certain cases (eg RSS hostname update from prefs page)
                        paramValue = strdup("");
                    } else {
                        paramValue = strdup(paramEquals + 1);
                    }
                }

				char* unescapedValue = unescapeValue(paramValue);
                struct NameValuePair* param = makeNameValuePair(paramName, unescapedValue);
                appendNameValuePair(&(request->params), param);

             // These are all malloced above, and copied by makeNameValuePair(), so free them here
                free(unescapedValue);
                free(paramName);
                free(paramValue);
            }
        }
    }

    char* headerName;
    char* headerValue;
    char* headers = strtok(requestTxt,HTTP_EOL);
    // Extract the headers one at a time, and store them
    while(1){
        headerName = strtok(NULL, ": ");
        headerValue = strtok(NULL, HTTP_EOL);

        if( headerName == NULL || headerValue == NULL ){
            break;
        } else {
            // Strip any leading whitespace
            while( *headerName == ' ' || *headerName == '\n' || *headerName == '\r' ) headerName++;
            while( *headerValue == ' ' ) headerValue++;
            struct NameValuePair* header = makeNameValuePair(headerName, headerValue);
            appendNameValuePair(&(request->headers), header);
        }
    }

    if (isLogInfo()){
        logRequest(request);
    }
    return request;
}

static void logRequest(struct Request* request){
    logMsg(LOG_INFO, "Parsed Request to: %s %s", request->method, request->path);
    struct NameValuePair* param = request->params;
    while (param != NULL){
        logMsg(LOG_INFO, "        %s=%s", param->name, param->value);
        param = param->next;
    }
    param = request->headers;
    while (param != NULL){
        logMsg(LOG_INFO, "        %s=%s", param->name, param->value);
        param = param->next;
    }
}

void freeRequest(struct Request* request){
 // Free up all the memory used by a Request struct
    if (request != NULL){
        if (request->method != NULL){
            free(request->method);
        }
        if (request->path != NULL){
            free(request->path);
        }
        freeNameValuePairs(request->params);
        freeNameValuePairs(request->headers);
        free(request);
    }
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
