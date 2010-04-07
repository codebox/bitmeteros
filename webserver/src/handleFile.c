/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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

#include <stdlib.h>
#ifndef _WIN32
#include <sys/socket.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "bmws.h"
#include "common.h"

/*
Handles requests for files received by the web server.
*/

extern struct HttpResponse HTTP_OK;
extern struct HttpResponse HTTP_NOT_FOUND;
extern struct HttpResponse HTTP_FORBIDDEN;
extern struct HttpResponse HTTP_NOT_ALLOWED;

struct MimeType{
	char* fileExt;
	char* contentType;
	int binary;
};

struct MimeType MIME_TYPES[] = {
	{"html", MIME_HTML, FALSE},
	{"htm",  MIME_HTML, FALSE},
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

static struct MimeType* getMimeTypeForFile(char* fileName){
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

	static long getFileSize(FILE* fp){
		return filelength(fileno(fp));
	}
#endif

#ifndef _WIN32
	static FILE* openFile(char* path, int binary){
		return fopen(path, binary ? "rb" : "r");
	}
	static long getFileSize(FILE* fp){
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		return size;
	}
#endif

void processFileRequest(SOCKET fd, struct Request* req){
    setupEnv();

	char* path = req->path;
 // Default page is index.html, send this if no other file is specified
	if (strcmp("/", path) == 0){
        //free(req->path);
        path = strdup("/index.html");
	}
	errno = 0;

	struct MimeType* mimeType = getMimeTypeForFile(path);
    FILE* fp = openFile(path, mimeType->binary);

    if (fp == NULL){
     // We couldn't get the file, find out why not and return an appropriate HTTP error
        struct HttpResponse response;
        if (errno == ENOENT){
            response = HTTP_NOT_FOUND;
        } else {
            response = HTTP_FORBIDDEN;
        }
        writeHeaders(fd, response, NULL, 0);

    } else {
     // We got the file, write out the headers and then send the content
	    //int size = getFileSize(fp); this was causing problems on Google Chrome so removed, dont think we actually need to send this
        writeHeaders(fd, HTTP_OK, mimeType->contentType, 0);

        int rc;
        char buffer[BUFSIZE];
        while ( (rc = fread(buffer, 1, BUFSIZE, fp)) > 0 ) {
            send(fd, buffer, rc, 0);
        }
        fclose(fp);
    }

}
