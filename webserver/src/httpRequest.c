#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include "bmws.h"
#include "common.h"

/*
Contains code for creating and processing Request structs.
*/

static void logRequest(struct Request* request);

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
        while (1) {
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
