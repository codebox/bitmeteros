/*
 * BitMeterOS v0.3.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
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
 *
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmws.h"
#include "common.h"

/*
Contains HTTP-related functions, mostly for reading in requests and writing out responses.
*/

extern struct HttpResponse HTTP_OK;
extern struct HttpResponse HTTP_NOT_ALLOWED;

// These are the different operations that we can perform on behalf of the client
enum OpType{File, Monitor, Summary, Query, Sync, Config};

static void writeHeader(SOCKET fd, char* name, char* value){
 // Helper function, writes out a single HTTP header with the appropriate separator and line terminator
	char buffer[HEADER_BUFSIZE];
    sprintf(buffer, "%s: %s" HTTP_EOL, name, value);
    writeText(fd, buffer);
}

static void writeMimeType(SOCKET fd, char* contentType){
	writeHeader(fd, HEADER_CONTENT_TYPE, contentType);
}

static void writeResponseCode(SOCKET fd, struct HttpResponse response){
 // Writes out the first line of a response, including the HTTP response code and message
    char buffer[strlen(response.msg) + 16]; // Accomodate code, HTTP prefix, whitespace etc
    sprintf(buffer,"HTTP/1.0 %d %s" HTTP_EOL, response.code, response.msg);
    writeText(fd, buffer);
}

static void writeContentLength(SOCKET fd, int length){
    char lenBuffer[16];
    sprintf(lenBuffer, "%d", length);
    writeHeader(fd, "Content-Length", lenBuffer);
}

static void writeCommonHeaders(SOCKET fd){
 // We send these out on every response
 	writeHeader(fd, "Server", "BitMeterOS " VERSION " Web Server");

    char dateTxt[32];
	time_t now = getTime();
	strftime(dateTxt, sizeof(dateTxt), "%a, %d %b %Y %H:%M:%S +0000", gmtime(&now));
	writeHeader(fd, "Date", dateTxt);

	writeHeader(fd, "Connection", "Close");
}

static void writeEndOfHeaders(SOCKET fd){
    char* buffer = HTTP_EOL;
    writeText(fd, buffer);
}

void writeHeaders(SOCKET fd, struct HttpResponse response, char* contentType, int size){
 // Writes out a full set of headers including the specified HTTP response and, if appropriate, the MIME type
    writeResponseCode(fd, response);

    if (response.code == HTTP_OK.code){
     // Only need this if we are returning some content
        writeMimeType(fd, contentType);
    }

    if (size > 0){
    	writeContentLength(fd, size);
    }

    writeCommonHeaders(fd);
    writeEndOfHeaders(fd);
}

void processRequest(SOCKET fd, char* buffer){
 // Examine the request, and hand it off to the appropriate handler
    struct Request* req = parseRequest(buffer);

    if (strcmp(req->method, "GET") == 0) {
        enum OpType op;

        if (strcmp(req->path, "/monitor") == 0){
            op = Monitor;

		} else if (strcmp(req->path, "/summary") == 0){
            op = Summary;

		} else if (strcmp(req->path, "/query") == 0){
            op = Query;

		} else if (strcmp(req->path, "/sync") == 0){
            op = Sync;

		} else if (strcmp(req->path, "/config") == 0){
            op = Config;

        } else {
            op = File;
        }

		#ifdef _WIN32
			waitForMutex();
		#endif

        int needsDb = (op != File);
        if (needsDb){
         // The client isn't asking for a file, so we will need a database connection to complete the request
            openDb();
            prepareDb();
        }

     // Call the appropriate request handler
        if (op == Monitor){
            processMonitorRequest(fd, req);

		} else if (op == Summary){
            processSummaryRequest(fd, req);

		} else if (op == Query){
            processQueryRequest(fd, req);

		} else if (op == Sync){
            processSyncRequest(fd, req);

		} else if (op == Config){
            processConfigRequest(fd, req);

        } else if (op == File){
            processFileRequest(fd, req);

        } else {
            assert(FALSE);
        }

        if (needsDb){
         // Clean up
            closeDb();
        }
		#ifdef _WIN32
			releaseMutex();
		#endif
    } else {
     // Only GET requests are allowed - send an error
        writeResponseCode(fd, HTTP_NOT_ALLOWED);
        writeHeader(fd, "Allow", "GET");
        writeEndOfHeaders(fd);
    }

    freeRequest(req);
}

void writeText(SOCKET fd, char* txt){
 // Helper function, computes the length of the text for us
    #ifdef TESTING
        write(fd, txt, strlen(txt));
        fsync(fd);
    #else
        send(fd, txt, strlen(txt), 0);
    #endif
}

void writeDataToJson(SOCKET fd, struct Data* data){
 // Converts the Data struct into an JSON array and writes it out to the stream
	writeText(fd, "[");
	int i=0;
	while (data != NULL) {
		if (i++ > 0){
			writeText(fd, ",");
		}
		writeSingleDataToJson(fd, data);
		data = data->next;
	}
	writeText(fd, "]");
}

void writeSingleDataToJson(SOCKET fd, struct Data* data){
	char jsonBuffer[64];

	writeText(fd, "{");

	if (data != NULL){
		sprintf(jsonBuffer, "dl: %llu,ul: %llu,ts: %d,dr: %d", data->dl, data->ul, (int)data->ts, data->dr);
		writeText(fd, jsonBuffer);
	}
	writeText(fd, "}");
}

void writeSyncData(SOCKET fd, struct Data* data){
	char row[64];
	sprintf(row, "%d,%d,%llu,%llu,%s" HTTP_EOL, (int)data->ts, data->dr, data->dl, data->ul, data->ad);
	writeText(fd, row);
}

#ifdef _WIN32
	HANDLE mutex = NULL;
	void initMutex(){
		assert(mutex == NULL);
		mutex = CreateMutex(NULL, FALSE, "Mutex");
	}
	void waitForMutex(){
		assert(mutex != NULL);
		WaitForSingleObject(mutex, INFINITE);
	}
	void releaseMutex(){
		assert(mutex != NULL);
		ReleaseMutex(mutex);
	}
#endif
