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
     	writeHeadersServerError(fd, "processSyncRequest ts param missing/invalid: %s", getValueForName("ts", req->params, NULL));

	} else {
	    writeHeadersOk(fd, SYNC_CONTENT_TYPE, TRUE);

        struct Data* results = getSyncValues(ts);
        struct Data* thisResult = results;

        while(thisResult != NULL){
            writeSyncData(fd, thisResult);
            thisResult = thisResult->next;
        }

        freeData(results);
	}
}

