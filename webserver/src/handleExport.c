/*
 * BitMeterOS
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
 */

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include "bmws.h"
#include "client.h"
#include "common.h"

/*
Handles '/export' requests received by the web server.
*/

extern struct HttpResponse HTTP_OK;
static void writeRow(struct Data* row, int fd);

void processExportRequest(SOCKET fd, struct Request* req){
    writeHeaders(fd, HTTP_OK, MIME_CSV, FALSE);
    writeHeader(fd, "Content-Disposition", "attachment;filename=bitmeterOsExport.csv");
	writeEndOfHeaders(fd);

	getDumpValues(&writeRow, fd);
}

static void writeRow(struct Data* row, int fd){
	char date[11];
	toDate(date, row->ts - row->dr);

 // This is the time marking the start of the interval
	char timeFrom[9];
	toTime(timeFrom, row->ts - row->dr);

 // This is the time marking the end of the interval
	char timeTo[9];
	toTime(timeTo, row->ts);

	char rowTxt[256];
	sprintf(rowTxt, "%s,%s,%s,%llu,%llu,%s,%s\n", date, timeFrom, timeTo, row->dl, row->ul, (row->hs == NULL) ? "" : row->hs, row->ad);	
	writeText(fd, rowTxt);
	
	freeData(row);
}

