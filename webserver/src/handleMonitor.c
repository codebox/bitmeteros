/*
 * BitMeterOS v0.3.2
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
 *
 * Build Date: Sun, 07 Mar 2010 14:49:47 +0000
 */

#include <stdio.h>
#include "client.h"
#include "bmws.h"
#include "common.h"

#define NO_TS -1

/*
Handles '/monitor' requests received by the web server.
*/

extern struct HttpResponse HTTP_OK;
extern struct HttpResponse HTTP_SERVER_ERROR;

void processMonitorRequest(SOCKET fd, struct Request* req){
	struct NameValuePair* params = req->params;

	int ts = getValueNumForName("ts", params, NO_TS);
	if (ts == NO_TS){
     // We need a 'ts' parameter
	    writeHeaders(fd, HTTP_SERVER_ERROR, NULL, 0);

	} else {
        writeHeaders(fd, HTTP_OK, MIME_JSON, 0);

     /* The 'ts' parameter is an offset from the current (server) time rather than an actual timestamp, done like this
        because there may be differences in the clocks on the client and server, if the client was a minute
        or two ahead of the server it would never get any data back. */
        int now = getTime();
        int queryTs = now - ts;

        struct Data* result = getMonitorValues(queryTs);
        
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
        sprintf(jsonBuffer, "{serverTime : %d, data : ", now);
        writeText(fd, jsonBuffer);
        writeDataToJson(fd, resultsFromNow);
        writeText(fd, "}");
        freeData(result);
	}
}
