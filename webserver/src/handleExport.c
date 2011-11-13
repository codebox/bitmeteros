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
static struct Filter* filters;

void processExportRequest(SOCKET fd, struct Request* req){
    filters = readFilters();
    
    WRITE_HEADERS_OK(fd, MIME_CSV, FALSE);
    WRITE_HEADER(fd, "Content-Disposition", "attachment;filename=bitmeterOsExport.csv");
	WRITE_END_OF_HEADERS(fd);

	getDumpValues(fd, (void (*)(int, struct Data*))&writeCsvRow);
	
	freeFilters(filters);
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

    struct Filter* filter = getFilterFromId(filters, row->fl);
    char* filterName;
    if (filter != NULL){
        filterName = filter->name;
    } else {
        filterName = "";
    }
	char rowTxt[256]; //TODO make this more useful, include filter name? and host
	sprintf(rowTxt, "%s,%s,%s,%llu,%s\n", date, timeFrom, timeTo, row->vl, filterName);	
	WRITE_TEXT(fd, rowTxt);
	
	freeData(row);
}