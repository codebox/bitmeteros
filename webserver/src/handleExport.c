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

static void writeCsvRow(SOCKET fd, struct Data* row);
	
void processExportRequest(SOCKET fd, struct Request* req){
    writeHeadersOk(fd, MIME_CSV, FALSE);
    writeHeader(fd, "Content-Disposition", "attachment;filename=bitmeterOsExport.csv");
	writeEndOfHeaders(fd);

	getDumpValues(fd, (void (*)(int, struct Data*))&writeCsvRow);
}

static void writeCsvRow(SOCKET fd, struct Data* row){
	char date[11];
	toDate(date, row->ts - row->dr);

 // This is the time marking the start of the interval
	char timeFrom[9];
	toTime(timeFrom, row->ts - row->dr);

 // This is the time marking the end of the interval
	char timeTo[9];
	toTime(timeTo, row->ts);

	char rowTxt[256]; //TODO make this more useful, include filter name? and host
	sprintf(rowTxt, "%s,%s,%s,%llu,%d\n", date, timeFrom, timeTo, row->vl, row->fl);	
	writeText(fd, rowTxt);
	
	freeData(row);
}

