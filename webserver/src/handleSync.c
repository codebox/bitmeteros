/*
 * BitMeterOS v0.2.0
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
 * Build Date: Wed, 25 Nov 2009 10:48:23 +0000
 */

#include <stdio.h>
#include <string.h>
#include "client.h"
#include "bmws.h"
#include "common.h"

#define SYNC_CONTENT_TYPE "application/vnd.codebox.bitmeter-sync"
#define NO_TS 0

/*
Handles '/sync' requests received by the web server.
*/

extern struct HttpResponse HTTP_OK;

static void writeSyncData(SOCKET fd, struct Data* data){
	char row[64];
	sprintf(row, "%d,%d,%llu,%llu,%s" HTTP_EOL, (int)data->ts, data->dr, data->dl, data->ul, data->ad);
	writeText(fd, row);
}

void processSyncRequest(SOCKET fd, struct Request* req){
	int ts = getValueNumForName("ts", req->params, NO_TS);
	if (ts == NO_TS){
            //TODO
	} else {
        int queryTs = getTime() - ts;

        struct Data* results = getSyncValues(queryTs);
        struct Data* thisResult = results;

        while(thisResult != NULL){
            writeSyncData(fd, thisResult);
            thisResult = thisResult->next;
        }

        freeData(results);
	}
}

