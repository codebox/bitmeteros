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

