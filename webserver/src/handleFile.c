#define _GNU_SOURCE
#include <stdlib.h>
#ifndef _WIN32
    #include <sys/socket.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include "bmws.h"
#include "common.h"

/*
Handles requests for files received by the web server.
*/

struct MimeType MIME_TYPES[] = {
    {"html", MIME_HTML, FALSE},
    {"htm",  MIME_HTML, FALSE},
    {"xml",  MIME_XML,  FALSE},
    {"jpeg", MIME_JPEG, TRUE},
    {"jpg",  MIME_JPEG, TRUE},
    {"gif",  MIME_GIF,  TRUE},
    {"png",  MIME_PNG,  TRUE},
    {"ico",  MIME_ICO,  TRUE},
    {"js",   MIME_JS,   FALSE},
    {"css",  MIME_CSS,  FALSE},
    {NULL,   NULL,      FALSE}
};
struct MimeType DEFAULT_MIME_TYPE = {"bin", MIME_BIN, TRUE};

struct MimeType* getMimeTypeForFile(char* fileName){
 // Work out which MIME type we should use, based on the name of the file
    char* dotPosn = strrchr(fileName,  '.');
    if (dotPosn != NULL){
        char* extPosn = dotPosn +1;

     // Check the file extension against the known types in the MIME_TYPES array
        struct MimeType* mimeTypeListItem = MIME_TYPES;
        while (mimeTypeListItem->fileExt != NULL){
            if (strcmp(mimeTypeListItem->fileExt, extPosn) == 0){
             // Found a match
                return mimeTypeListItem;
            }
            mimeTypeListItem++;
        }
        return &DEFAULT_MIME_TYPE;

    } else {
     // This doesn't look like a file name
        return &DEFAULT_MIME_TYPE;
    }
}

#ifdef _WIN32
    static void setupEnv(){}
#endif

#ifndef _WIN32
    static void setupEnv(){
     // Make sure we are correctly set up in a chroot jail before looking for the file
        char webRoot[MAX_PATH_LEN];
        getWebRoot(webRoot);

        int rc = chdir(webRoot);
        if (rc < 0){
            logMsg(LOG_ERR, "chdir(%s) returned %d, %s", webRoot, rc, strerror(errno));
            exit(1);
        }

        rc = chroot(webRoot);
        if (rc < 0){
            logMsg(LOG_ERR, "chroot(%s) returned %d, %s", webRoot, rc, strerror(errno));
            exit(1);
        }
    }
#endif

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <shlwapi.h>

    static FILE* openFile(char* path, int binary){
     /* No such thing as chroot in Windows so we need to do some checks to ensure the file that
        has been requested is inside the designated web root folder. */
        int i, length = strlen(path);
        for (i=0; i<length; i++){
         // Change the forward-slashes to backslashes
            if (path[i] == '/'){
                path[i] = '\\';
            }
        }

        char filePath[MAX_PATH];
        getWebRoot(filePath);
        char *webPath = strdupa(filePath);

     // Find the absolute path of the requested file
        PathAppend(filePath, TEXT((char *)(path + 1)));

     // Canonicalise the path, to eliminate any trickery using '../..'
        char canonicalPath[MAX_PATH];
        int rc = PathCanonicalize(canonicalPath, filePath);

        if (rc == TRUE){
         // If the canonical path starts with the path of the web root then we are happy the request should be allowed
            if (strncmp(canonicalPath, webPath, strlen(webPath)) == 0){
                return fopen(canonicalPath, binary ? "rb" : "r");
            } else {
                logMsg(LOG_ERR, "Possible folder-traversal attack, request was %s which is not inside the web root of %s", path, webPath);
                return NULL;
            }
        } else {
            logMsg(LOG_ERR, "Unable to canonicalize the file path %s", filePath);
            return NULL;
        }

    }

#endif

#ifndef _WIN32
    static FILE* openFile(char* path, int binary){
        return fopen(path, binary ? "rb" : "r");
    }
#endif

void doSubs(SOCKET fd, FILE* fp, struct NameValuePair* substPairs){
    char bufferIn[SUBST_BUFSIZE];
    char bufferOut[SUBST_BUFSIZE];
    int size = FREAD(bufferIn, 1, sizeof(bufferIn), fp);
    
    if (size == SUBST_BUFSIZE){
        logMsg(LOG_ERR, "doSubs, file too large - exceeded %d bytes", size);
        
    } else {
        bufferIn[size] = 0;
        char marker[32];
        while (substPairs != NULL) {
            sprintf(marker, "<!--[%s]-->", substPairs->name);

            char* match;
            int matchLen, valueLen, bufferInLen, matchOffset;
            while ((match = strstr(bufferIn, marker)) != NULL){
                bufferInLen = strlen(bufferIn);
                matchLen = strlen(marker);
                valueLen = strlen(substPairs->value);
                matchOffset = match - bufferIn;
                
                if (bufferInLen - matchLen + valueLen >= SUBST_BUFSIZE){
                    logMsg(LOG_ERR, "doSubs, file too large after substitution of value %s - max is %d bytes", substPairs->value, size);
                    break;
                }
                
             // Copy everything before the start of the marker
                strncpy(bufferOut, bufferIn, matchOffset);
                
             // Copy the value in place of the marker
                strncpy(bufferOut + matchOffset, substPairs->value, valueLen);
                
             // Copy the remainder of the string, after the match, including the trailing null byte
                strncpy(bufferOut + matchOffset + valueLen, bufferIn + matchOffset + matchLen, bufferInLen - matchOffset - matchLen + 1);
                
             // Copy bufferOut into bufferIn and start again
                size = strlen(bufferOut);
                strncpy(bufferIn, bufferOut, size);
                bufferIn[size] = 0;
            }
            
            substPairs = substPairs->next;  
        }

        WRITE_DATA(fd, bufferIn, size);
    }
}

void processFileRequest(SOCKET fd, struct Request* req, struct NameValuePair* substPairs){
    setupEnv();

    int redirect = 0;
    char* path = req->path;
 // Default page is index.html, send this if no other file is specified
    if (strcmp("/", path) == 0){
        redirect = 1;
        //free(req->path);
        path = strdup("/index.html");
        
    } else if ((strcmp("/m", path) == 0) || (strcmp("/m/", path) == 0)){
        redirect = 1;
        //free(req->path);
        path = strdup("/m/index.xml");

    } else if ((strncmp(path, "/m/", 3) == 0) && (strchr(path, '.') == NULL)){
        int newPathLen = strlen(path) + 5;
        char tmp[newPathLen];
        sprintf(tmp, "%s.xml", path);
        tmp[newPathLen] = 0;
        path = strdup(tmp);
    }
    errno = 0;

    struct MimeType* mimeType = getMimeTypeForFile(path);
    FILE* fp = openFile(path, mimeType->binary);

    if (fp == NULL){
     // We couldn't get the file, find out why not and return an appropriate HTTP error
        struct HttpResponse response;
        if (errno == ENOENT){
            writeHeadersNotFound(fd, path);
        } else {
            writeHeadersForbidden(fd, "file access");
        }

    } else {
        // We got the file, write out the headers and then send the content
        if ( redirect ){
            // Was the initial request only for "/" ?
            struct NameValuePair* param = req->headers;
            while (param != NULL){
                if (strcmp(param->name, "Host") == 0 ) {
                    writeHeadersSeeOther(fd, req, TRUE);
                }
                param = param->next;
            }
        } else {
            writeHeadersOk(fd, mimeType->contentType, TRUE);
        }
        if (substPairs == NULL){
            int rc;
            char buffer[BUFSIZE];
            while ( (rc = FREAD(buffer, 1, BUFSIZE, fp)) > 0 ) {
                   WRITE_DATA(fd, buffer, rc);
            }
        } else {
            doSubs(fd, fp, substPairs);
        }
        fclose(fp);
    }

}
