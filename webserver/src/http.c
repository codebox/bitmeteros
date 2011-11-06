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

static struct HttpResponse HTTP_OK           = {200, "OK"};
static struct HttpResponse HTTP_NOT_FOUND    = {404, "Not Found"};
static struct HttpResponse HTTP_FORBIDDEN    = {403, "Forbidden"};
static struct HttpResponse HTTP_NOT_ALLOWED  = {405, "Method not allowed"};
static struct HttpResponse HTTP_SERVER_ERROR = {500, "Bad/missing parameter"};

static void writeHeaders(SOCKET fd, struct HttpResponse response, char* contentType, int endHeaders);
    
// These are the different operations that we can perform on behalf of the client
enum OpType{File, Monitor, Summary, Query, Sync, Config, Alert, RSS, MobileMonitor, MobileSummary, MobileAbout};

void writeHeader(SOCKET fd, char* name, char* value){
 // Helper function, writes out a single HTTP header with the appropriate separator and line terminator
    char buffer[SMALL_BUFSIZE];
    sprintf(buffer, "%s: %s" HTTP_EOL, name, value);
    writeText(fd, buffer);
}

static void writeMimeType(SOCKET fd, char* contentType){
    writeHeader(fd, HEADER_CONTENT_TYPE, contentType);
}

static void writeResponseCode(SOCKET fd, struct HttpResponse response){
 // Writes out the first line of a response, including the HTTP response code and message
    char buffer[SMALL_BUFSIZE];
    sprintf(buffer,"HTTP/1.0 %d %s" HTTP_EOL, response.code, response.msg);
    writeText(fd, buffer);
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

void writeEndOfHeaders(SOCKET fd){
    char* buffer = HTTP_EOL;
    writeText(fd, buffer);
}

void writeHeadersNotFound(SOCKET fd, char* file){
    logMsg(LOG_ERR, "%s: %s", HTTP_NOT_FOUND.msg, file);
    writeHeaders(fd, HTTP_NOT_FOUND, NULL, TRUE);
}

void writeHeadersForbidden(SOCKET fd, char* request){
    logMsg(LOG_ERR, "%s: %s", HTTP_FORBIDDEN.msg, request);
    writeHeaders(fd, HTTP_FORBIDDEN, NULL, TRUE);
}

void writeHeadersNotAllowed(SOCKET fd, char* httpMethod){
    logMsg(LOG_ERR, "%s: %s", HTTP_NOT_ALLOWED.msg, httpMethod);
    writeHeaders(fd, HTTP_NOT_ALLOWED, NULL, FALSE);
}

void writeHeadersServerError(SOCKET fd, char* msg, ...){
    va_list argp;
    va_start(argp, msg);
    vlogMsg(LOG_ERR, msg, argp);
    va_end(argp);
    writeHeaders(fd, HTTP_SERVER_ERROR, NULL, TRUE);
}

void writeHeadersOk(SOCKET fd, char* contentType, int endHeaders){
    writeHeaders(fd, HTTP_OK, contentType, endHeaders);
}

static void writeHeaders(SOCKET fd, struct HttpResponse response, char* contentType, int endHeaders){
 // Writes out a full set of headers including the specified HTTP response and, if appropriate, the MIME type
    writeResponseCode(fd, response);

    if (response.code == HTTP_OK.code && contentType != NULL){
     // Only need this if we are returning some content
        writeMimeType(fd, contentType);
    }

    writeCommonHeaders(fd);
    
    if (endHeaders){
        writeEndOfHeaders(fd);
    }
}

void processRequest(SOCKET fd, char* buffer, int allowAdmin){
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

        } else if (strcmp(req->path, "/alert") == 0){
            op = Alert;

        } else if (strcmp(req->path, "/rss.xml") == 0){
            op = RSS;

        } else if (strcmp(req->path, "/m/monitor") == 0) {
            op = MobileMonitor;
            
        } else if (strcmp(req->path, "/m/summary") == 0) {
            op = MobileSummary;
            
        } else if (strcmp(req->path, "/m/about") == 0) {
            op = MobileAbout;
            
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
            processConfigRequest(fd, req, allowAdmin);

        } else if (op == Alert){
            processAlertRequest(fd, req, allowAdmin);
            
        } else if (op == RSS){
            processRssRequest(fd, req);

        } else if (op == MobileMonitor){
            processMobileMonitorRequest(fd, req);
            
        } else if (op == MobileSummary){
            processMobileSummaryRequest(fd, req);
            
        } else if (op == MobileAbout){
            struct NameValuePair pair = {"version", VERSION, NULL};
            processFileRequest(fd, req, &pair);

        } else if (op == File){
            processFileRequest(fd, req, NULL);

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
        writeHeadersNotAllowed(fd, req->method);
        writeHeader(fd, "Allow", "GET");
        writeEndOfHeaders(fd);
    }

    freeRequest(req);
}

void writeText(SOCKET fd, char* txt){
 // Helper function, computes the length of the text for us
    writeData(fd, txt, strlen(txt));
}

void writeData(SOCKET fd, char* data, int len){
    #ifdef TESTING
        write(fd, data, len);
        #ifndef _WIN32
            fsync(fd);
        #endif
    #else
        send(fd, data, len, 0);
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
    char jsonBuffer[SMALL_BUFSIZE];

    writeText(fd, "{");

    if (data != NULL){
        sprintf(jsonBuffer, "\"vl\": %llu,\"ts\": %d,\"dr\": %d,\"fl\": %d", data->vl, (int)data->ts, data->dr, data->fl);
        writeText(fd, jsonBuffer);
    }
    writeText(fd, "}");
}
void writeTextArrayToJson(SOCKET fd, char* key, char** values){
    if (key != NULL){
        writeText(fd, "\"");
        writeText(fd, key);
        writeText(fd, "\" : ");
    }
    writeText(fd, "[");
    
    int firstItem = TRUE;
    while (*values != NULL){
        if (!firstItem) {
            writeText(fd, ",");
        }
        firstItem = FALSE;
        
        writeText(fd, "\"");
        writeText(fd, *values);
        writeText(fd, "\"");
        values++;
    }
    
    writeText(fd, "]");

}
void writeTextValueToJson(SOCKET fd, char* key, char* value){
    char jsonBuffer[SMALL_BUFSIZE];
    
    if (strlen(key) + strlen(value) + 8 > SMALL_BUFSIZE){
        logMsg(LOG_ERR, "Input values too large for buffer, key='%s' value='%s'", key, value);      
    } else {
        sprintf(jsonBuffer, "\"%s\" : \"%s\"", key, value);
        writeText(fd, jsonBuffer);
    }
}
void writeNumValueToJson(SOCKET fd, char* key, BW_INT value){
    char jsonBuffer[SMALL_BUFSIZE];
    
    if (strlen(key) + 40 > SMALL_BUFSIZE) {
        logMsg(LOG_ERR, "Input values too large for buffer, key='%s' value=%llu", key, value);      
    } else {
        sprintf(jsonBuffer, "\"%s\" : %llu", key, value);
        writeText(fd, jsonBuffer);
    }
}

void writeSyncData(SOCKET fd, struct Data* data){
    char row[SMALL_BUFSIZE];
    sprintf(row, "%d,%d,%llu,%d" HTTP_EOL, (int)data->ts, data->dr, data->vl, data->fl);
    writeText(fd, row);
}

void writeFilterData(SOCKET fd, struct Filter* filter){
    char row[BUFSIZE];
    sprintf(row, FILTER_ROW_PREFIX "%d,%s,%s,%s" HTTP_EOL, filter->id, filter->name, filter->desc, filter->expr);
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
