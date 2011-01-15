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

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include "client.h"
#include "bmws.h"
#include "common.h"

#define NO_TS -1
#define MOBILE_MONITOR_DEFAULT_TS 5

/*
Handles '/monitor' requests received by the web server.
*/

static void processMonitorAjaxRequest(SOCKET fd, int ts, char* ha){
	writeHeadersOk(fd, MIME_JSON, TRUE);

 /* The 'ts' parameter is an offset from the current (server) time rather than an actual timestamp, done like this
    because there may be differences in the clocks on the client and server, if the client was a minute
    or two ahead of the server it would never get any data back. */
    int now = getTime();
    int queryTs = now - ts;

    char* hs = NULL;
    char* ad = NULL;
    struct HostAdapter* hostAdapter = NULL;

    if (ha != NULL) {
        hostAdapter = getHostAdapter(ha);
        hs = hostAdapter->host;
        ad = hostAdapter->adapter;
    }

    struct Data* result = getMonitorValues(queryTs, hs, ad);

    if (ha != NULL) {
        freeHostAdapter(hostAdapter);
    }

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
    writeText(fd, jsonBuffer);
    writeDataToJson(fd, resultsFromNow);
    writeText(fd, "}");
    freeData(result);
}

void processMonitorRequest(SOCKET fd, struct Request* req){
	struct NameValuePair* params = req->params;

	int   ts = getValueNumForName("ts", params, NO_TS);
	char* ha = getValueForName("ha", params, NULL);

	if (ts == NO_TS){
     // We need a 'ts' parameter
     	writeHeadersServerError(fd, "processMonitorRequest, ts parameter missing/invalid: %s", 
     			getValueForName("ts", params, NULL));

	} else {
        processMonitorAjaxRequest(fd, ts, ha);
        
	}
}

void processMobileMonitorRequest(SOCKET fd, struct Request* req){
	int ts = getValueNumForName("ts", req->params, NO_TS);
	if (ts == NO_TS){
	 // No 'ts' param means that this was a full page request
		struct Data* result = getMonitorValues(getTime() - MOBILE_MONITOR_DEFAULT_TS, NULL, NULL);
	
		BW_INT dlTotal = 0;
		BW_INT ulTotal = 0;
		
		while (result != NULL) {
			dlTotal += result->dl;	
			ulTotal += result->ul;
			result = result->next;
		}
	
		char dlTxt[32];
		char ulTxt[32];
		formatAmounts(dlTotal/MOBILE_MONITOR_DEFAULT_TS, ulTotal/MOBILE_MONITOR_DEFAULT_TS, dlTxt, ulTxt, UNITS_ABBREV);
		
	    struct NameValuePair pair1 = {"dl", dlTxt, NULL};
		struct NameValuePair pair2 = {"ul", ulTxt, &pair1};
	    processFileRequest(fd, req, &pair2);
	    
	} else {
	 // A 'ts' param means that this was an AJAX request
		processMonitorAjaxRequest(fd, ts, NULL);
	}
}

