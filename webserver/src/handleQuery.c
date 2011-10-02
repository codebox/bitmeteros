#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <stdlib.h>
#include "sqlite3.h"
#include "bmws.h"
#include "common.h"
#include "client.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define BAD_PARAM -1

/*
Handles '/query' requests received by the web server.
*/


static void writeCsvRow(SOCKET fd, struct Data* row, struct Filter* filters);

static struct HandleQueryCalls calls = {&writeHeadersServerError, &writeHeadersOk, &writeHeader,
	&writeEndOfHeaders, &writeDataToJson, &writeText};
                                         
static struct HandleQueryCalls getCalls(){
	#ifdef UNIT_TESTING	
		return mockHandleQueryCalls;
	#else
		return calls;
	#endif
}

void processQueryRequest(SOCKET fd, struct Request* req){
	struct NameValuePair* params = req->params;

	time_t from  = (time_t) getValueNumForName("from",  params,  BAD_PARAM);
	time_t to    = (time_t) getValueNumForName("to",    params,  BAD_PARAM);
    long   group = getValueNumForName("group", params,  BAD_PARAM);
	int*   fl    = getNumListForName("fl", params);
	int    csv   = getValueNumForName("csv", params, FALSE);

    if (from == BAD_PARAM || to == BAD_PARAM || group == BAD_PARAM || fl == NULL){
     // We need all 4 parameters
     	getCalls().writeHeadersServerError(fd, "processQueryRequest, param bad/missing from=%s, to=%s, group=%s, fl=%d",
     		getValueForName("from",  params, NULL),
     		getValueForName("to",    params, NULL),
     		getValueForName("fl",    params, NULL),
     		getValueForName("group", params, NULL));

    } else {
        if (from > to){
         // Allow from/to values in either order
            time_t tmp = from;
            from = to;
            to = tmp;
        }

     /* The client will send the last date that should be included in the query range in the 'to' parameter. When
        computing the timestamp value that corresponds to this, we need to move to the end of the specified date to
        be sure of including all data transferred during that day. */
        struct tm* cal = localtime(&to);
        cal->tm_mday++;
        to = mktime(cal);

		int* thisFl = fl;
		struct Data* result = NULL;
	    while(*thisFl>0){
	    	appendData(&result, getQueryValues(from, to, group, *thisFl));
	    	thisFl++;
	    }

		if (csv){
		 // Export the query results in CSV format
		    getCalls().writeHeadersOk(fd, MIME_CSV, FALSE);
		    getCalls().writeHeader(fd, "Content-Disposition", "attachment;filename=bitmeterOsQuery.csv");
		    getCalls().writeEndOfHeaders(fd);
		    
		    struct Data* thisResult = result;
		    
			struct Filter* filters = readFilters();

		    while(thisResult != NULL){
		    	writeCsvRow(fd, thisResult, filters);	
		    	thisResult = thisResult->next;	
		    }
		    freeFilters(filters);
		    
		} else {
		 // Send results back as JSON
	        getCalls().writeHeadersOk(fd, MIME_JSON, TRUE);
			getCalls().writeDataToJson(fd, result);	
		}
        
        freeData(result);
        free(fl);
    }

}

static void writeCsvRow(SOCKET fd, struct Data* row, struct Filter* filters){
	char datePart[11];
	toDate(datePart, row->ts - row->dr);

	char timePart[9];
	toTime(timePart, row->ts - row->dr);

	char rowTxt[256]; //TODO long filter names will break this
	struct Filter* filter = getFilterFromId(filters, row->fl);
	
	sprintf(rowTxt, "%s %s,%llu,%s\n", datePart, timePart, row->vl, filter->name);	
	getCalls().writeText(fd, rowTxt);
}