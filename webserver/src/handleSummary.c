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
#include "bmws.h"
#include "client.h"
#include "common.h"

/*
Handles '/summary' requests received by the web server.
*/

static void writeTotal(SOCKET, char*, BW_INT, BW_INT);
extern struct HttpResponse HTTP_OK;

void processSummaryRequest(SOCKET fd, struct Request* req){
    writeHeaders(fd, HTTP_OK, MIME_JSON, 0);

	struct Summary summary = getSummaryValues();

	writeText(fd, "{");
	writeTotal(fd, "today", summary.today->dl, summary.today->ul);
	writeText(fd, ", ");
	writeTotal(fd, "month", summary.month->dl, summary.month->ul);
	writeText(fd, ", ");
	writeTotal(fd, "year",  summary.year->dl,  summary.year->ul);
	writeText(fd, ", ");
	writeTotal(fd, "total", summary.total->dl, summary.total->ul);
	writeText(fd, ", ");
	writeText(fd, "hosts: [");

	int i;
	for(i=0; i<summary.hostCount; i++){
	    if (i>0){
            writeText(fd, ", ");
	    }
	    writeText(fd, "'");
	    writeText(fd, summary.hostNames[i]);
	    writeText(fd, "'");
	}

	writeText(fd, "], ");
	writeText(fd, "since: ");

	char sinceTs[12];
    unsigned long since = (unsigned long) summary.tsMin;
	sprintf(sinceTs, "%lu", since);
	writeText(fd, sinceTs);
	writeText(fd, "}");

 // Free memory
	freeData(summary.today);
	freeData(summary.month);
	freeData(summary.year);
	freeData(summary.total);
}

static void writeTotal(SOCKET fd, char* totalName, BW_INT dl, BW_INT ul){
 // Helper function, writes out one of the summary totals in the correct JSON format
 	char prefix[32];
 	sprintf(prefix, "%s: ", totalName);
	writeText(fd, prefix);

	struct Data* data = allocData();
	data->dl = dl;
	data->ul = ul;

	writeSingleDataToJson(fd, data);
	freeData(data);
}

