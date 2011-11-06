#ifdef _WIN32
    #define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <string.h>
#include "client.h"
#include "bmws.h"
#include "common.h"

#define NO_TS -1

/*
Handles '/sync' requests received by the web server.
*/

void processSyncRequest(SOCKET fd, struct Request* req){
    time_t ts = (time_t) getValueNumForName("ts", req->params, NO_TS);
    if (ts == NO_TS){
     // We need a 'ts' parameter
        WRITE_HEADERS_SERVER_ERROR(fd, "processSyncRequest ts param missing/invalid: %s", getValueForName("ts", req->params, NULL));

    } else {
        WRITE_HEADERS_OK(fd, SYNC_CONTENT_TYPE, TRUE);
        struct Filter* filters = readFilters();
        struct Filter* thisFilter = filters;
        while(thisFilter != NULL){
            if (thisFilter->host == NULL){
             // We only send local filters
                WRITE_FILTER_DATA(fd, thisFilter);
            }
            thisFilter = thisFilter->next;
        }
        freeFilters(filters);

        struct Data* results = getSyncValues(ts);
        struct Data* thisResult = results;

        while(thisResult != NULL){
            WRITE_SYNC_DATA(fd, thisResult);
            thisResult = thisResult->next;
        }

        freeData(results);
        
    }
}

