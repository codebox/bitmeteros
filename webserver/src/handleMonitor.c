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

static void processMonitorAjaxRequest(SOCKET fd, int ts, int* fl){
	WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);

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
    WRITE_TEXT(fd, jsonBuffer);
    WRITE_DATA_TO_JSON(fd, resultsFromNow);
    WRITE_TEXT(fd, "}");
    freeData(result);
}

void processMonitorRequest(SOCKET fd, struct Request* req) {
	struct NameValuePair* params = req->params;

	int  ts = getValueNumForName("ts", params, NO_VAL);
	int* fl = getNumListForName("fl", params);

	if (ts == NO_VAL) {
     // We need a 'ts' parameter
     	WRITE_HEADERS_SERVER_ERROR(fd, "processMonitorRequest, ts parameter missing/invalid: %s", 
     			getValueForName("ts", params, NULL));
     			
     } else if (fl == NULL){
     // We need a 'fl' parameter
     	WRITE_HEADERS_SERVER_ERROR(fd, "processMonitorRequest, fl parameter missing");
     	
     } else {
		processMonitorAjaxRequest(fd, ts, fl);
		free(fl);
        
	}
}

struct NameValuePair* buildFilterPairs(struct Data* data){
	struct NameValuePair* pairs = NULL;   
	struct Filter* filters = readFilters();
	
 // Initialise one pair for each filter, with an initial value of '0'
	struct Filter* filter = filters;
	while(filter != NULL){
		struct NameValuePair* pair = makeNameValuePair(filter->name, "0");
		appendNameValuePair(&pairs, pair);
		filter = filter->next;
	}
	
 /* In the first loop we iterate through all the Data items and accumulate a total
 	for each of the filters in 'pairs'. */
	while (data != NULL) {
		struct Filter* filter = getFilterFromId(filters, data->fl);
		if (filter==NULL){
			logMsg(LOG_ERR, "Unknown filterid %d passed into buildFilterPairs()", data->fl);
			
		} else {
			char* filterName = filter->name;
			
			struct NameValuePair* pair = getPairForName(filterName, pairs);
			BW_INT currentVal = strToBwInt(pair->value, 0);
			BW_INT newVal = currentVal + data->vl;
			
			free(pair->value);
			char amount[32];
			sprintf(amount, "%llu", newVal);
			
			pair->value = strdup(amount);
		}
		
		data = data->next;
	}
	
 /* In the second loop we iterate over 'pairs' and replace the numeric char* values 
 	with formatted versions of the same amounts. */
	struct NameValuePair* pair = pairs;
	while(pair != NULL){
		char formattedAmount[16];
		BW_INT val = strToBwInt(pair->value, 0);
		formatAmount(val, TRUE, TRUE, formattedAmount);
		free(pair->value);
		pair->value = strdup(formattedAmount);
		pair = pair->next;	
	}
	
	freeFilters(filters);
	return pairs;
}

char* makeHtmlFromData(struct NameValuePair* pair){
	//TODO move out of here into makeMobileMarkup.c
	char* html = strdup("<table id='monitor'>");
	char* tmp;
	
	while(pair != NULL){
		tmp = strAppend(html, "<tr><td class='filter'>", pair->name, "</td><td class='amt'>", pair->value, "/s</td></tr>", NULL);
		free(html);
		html = tmp;
		pair = pair->next;	
	}
	tmp = strAppend(html, "</table>", NULL);
	free(html);
	html = tmp;
	
	return html;
}

void processMobileMonitorRequest(SOCKET fd, struct Request* req){
	int ts = getValueNumForName("ts", req->params, NO_VAL);

	if (ts == NO_VAL){
	 // No 'ts' param means that this was a mobile full-page request
		struct Data* result = getMonitorValues(getTime() - MOBILE_MONITOR_DEFAULT_TS, NULL);
	
	 // Items in this list have: name=<filter name> value=<formatted total for filter>
		struct NameValuePair* pairs = buildFilterPairs(result);
		
		char* html = makeHtmlFromData(pairs);
		freeNameValuePairs(pairs);
		
	    struct NameValuePair pair = {"monitor", html, NULL};
	    processFileRequest(fd, req, &pair);
	    free(html);
	    
	} else {
	 // A 'ts' param means that this was an AJAX request
		processMonitorAjaxRequest(fd, ts, NULL);
	}
}

