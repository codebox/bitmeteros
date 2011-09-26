#ifdef UNIT_TESTING
	#include "test.h"
#endif
#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include "client.h"
#include "bmws.h"
#include "common.h"

#define NO_VAL -1
#define MOBILE_MONITOR_DEFAULT_TS 5

/*
Handles '/monitor' requests received by the web server.
*/

static struct HandleMonitorCalls calls = {&writeHeadersServerError, &writeHeadersOk, &writeDataToJson, &writeText};
                                         
static struct HandleMonitorCalls getCalls(){
	#ifdef UNIT_TESTING	
		return mockHandleMonitorCalls;
	#else
		return calls;
	#endif
}

static void processMonitorAjaxRequest(SOCKET fd, int ts, int* fl){
	getCalls().writeHeadersOk(fd, MIME_JSON, TRUE);

 /* The 'ts' parameter is an offset from the current (server) time rather than an actual timestamp, done like this
    because there may be differences in the clocks on the client and server, if the client was a minute
    or two ahead of the server it would never get any data back. */
    int now = getTime();
    int queryTs = now - ts;

    struct Data* result = NULL;
    while(*fl>0){
    	appendData(&result, getMonitorValues(queryTs, *fl));
    	fl++;
    }
    //struct Data* result = getMonitorValues(queryTs, fl);

 /* The database may contain values with timestamps that lie in the future, relative to the current system time. This
    can happen as a result of changes to/from GMT, or if the system clock is altered manually or accidentally. We don't
    want to return values with future timestamps, so move through the result list until we find a timestamp <= the
    current time. */
    struct Data* resultsFromNow = result;
	while((resultsFromNow != NULL) && (resultsFromNow->ts > now)){
		resultsFromNow = resultsFromNow->next;
	}

 // Change the 'ts' values in the response so that they contain the timestamp offset from the current time
    struct Data* curr = resultsFromNow;
    while(curr != NULL){
        curr->ts = (now - curr->ts);
        curr = curr->next;
    }

    char jsonBuffer[64];
    sprintf(jsonBuffer, "{\"serverTime\" : %d, \"data\" : ", now);
    getCalls().writeText(fd, jsonBuffer);
    getCalls().writeDataToJson(fd, resultsFromNow);
    getCalls().writeText(fd, "}");
    freeData(result);
}

void processMonitorRequest(SOCKET fd, struct Request* req) {
	struct NameValuePair* params = req->params;

	int  ts = getValueNumForName("ts", params, NO_VAL);
	int* fl = getNumListForName("fl", params);

	if (ts == NO_VAL) {
     // We need a 'ts' parameter
     	getCalls().writeHeadersServerError(fd, "processMonitorRequest, ts parameter missing/invalid: %s", 
     			getValueForName("ts", params, NULL));
     			
     } else if (fl == NULL){
     // We need a 'fl' parameter
     	getCalls().writeHeadersServerError(fd, "processMonitorRequest, fl parameter missing");
     	
     } else {
		processMonitorAjaxRequest(fd, ts, fl);
		free(fl);
        
	}
}

void processMobileMonitorRequest(SOCKET fd, struct Request* req){
	int ts = getValueNumForName("ts", req->params, NO_VAL);
	int fl = getValueNumForName("fl", req->params, NO_VAL);
	//TODO if no fl then error
	if (ts == NO_VAL){
	 // No 'ts' param means that this was a full page request
		struct Data* result = getMonitorValues(getTime() - MOBILE_MONITOR_DEFAULT_TS, fl);
	
		BW_INT total = 0;
		
		while (result != NULL) {
			total += result->vl;
			result = result->next;
		}
	
		char txt[32];
		formatAmount(total/MOBILE_MONITOR_DEFAULT_TS, TRUE, TRUE, txt);
		
	    struct NameValuePair pair = {"vl", txt, NULL};
	    processFileRequest(fd, req, &pair);
	    
	} else {
	 // A 'ts' param means that this was an AJAX request
		processMonitorAjaxRequest(fd, ts, NULL);
	}
}

