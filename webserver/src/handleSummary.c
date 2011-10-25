#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include "bmws.h"
#include "client.h"
#include "common.h"

/*
Handles '/summary' requests received by the web server.
*/

static void writeTotal(SOCKET, char*, struct Data*);

static struct HandleSummaryCalls calls = {&writeHeadersOk, &writeText, &writeNumValueToJson};

static struct HandleSummaryCalls getCalls(){
	#ifdef UNIT_TESTING	
		return mockHandleSummaryCalls;
	#else
		return calls;
	#endif
}

void processSummaryRequest(SOCKET fd, struct Request* req){
    getCalls().writeHeadersOk(fd, MIME_JSON, TRUE);

	struct Summary summary = getSummaryValues();

	getCalls().writeText(fd, "{");
	writeTotal(fd, "today", summary.today);
	getCalls().writeText(fd, ", ");
	writeTotal(fd, "month", summary.month);
	getCalls().writeText(fd, ", ");
	writeTotal(fd, "year",  summary.year);
	getCalls().writeText(fd, ", ");
	writeTotal(fd, "total", summary.total);
	getCalls().writeText(fd, ", ");

	if (summary.hostNames != NULL){
        getCalls().writeText(fd, "\"hosts\": [");

        int i;
        for(i=0; i<summary.hostCount; i++){
            if (i>0){
                getCalls().writeText(fd, ", ");
            }
            getCalls().writeText(fd, "\"");
            getCalls().writeText(fd, summary.hostNames[i]);
            getCalls().writeText(fd, "\"");
        }

        getCalls().writeText(fd, "]");
	} else {
	    getCalls().writeText(fd, "\"hosts\": null");
	}

	getCalls().writeText(fd, ", \"since\": ");

	char sinceTs[12];
    unsigned long since = (unsigned long) summary.tsMin;
	sprintf(sinceTs, "%lu", since);
	getCalls().writeText(fd, sinceTs);
	getCalls().writeText(fd, "}");

	freeSummary(&summary);
}

static void writeTotal(SOCKET fd, char* totalName, struct Data* data){
 // Helper function, writes out one of the summary totals in the correct JSON format
 	char prefix[32];
 	sprintf(prefix, "\"%s\": ", totalName);
	getCalls().writeText(fd, prefix);
	
	getCalls().writeText(fd, "[");
	int firstItem = TRUE;
	while(data != NULL){
		if (!firstItem){
			getCalls().writeText(fd, ",");	
		}

		getCalls().writeText(fd, "{");
		getCalls().writeNumValueToJson(fd, "vl", data->vl);
		getCalls().writeText(fd, ",");
		getCalls().writeNumValueToJson(fd, "fl", data->fl);
		getCalls().writeText(fd, "}");
		firstItem = FALSE;
		
		data = data->next;	
	}
	
	getCalls().writeText(fd, "]");
}

static void formatForMobile(BW_INT amt, char* txt){
	formatAmount(amt, TRUE, UNITS_ABBREV, txt);
}

static BW_INT getValueForFilter(int filterId, struct Data* data){
	BW_INT total = 0;
	while(data != NULL){
		if (data->fl == filterId){
			total += data->vl;	
		}
		data = data->next;
	}
	return total;
}
void processMobileSummaryRequest(SOCKET fd, struct Request* req){
	struct Summary summary = getSummaryValues();
	struct Filter* filters = readFilters();
	struct Filter* filter;
	
	char* html = strdup("");
	char* tmp;
	
 	filter = filters;
 	while(filter != NULL){
 		BW_INT today = getValueForFilter(filter->id, summary.today);
 		char todayTxt[32];
 		formatForMobile(today, todayTxt);
 		
 		BW_INT month = getValueForFilter(filter->id, summary.month);
 		char monthTxt[32];
 		formatForMobile(month, monthTxt);

 		BW_INT year  = getValueForFilter(filter->id, summary.year);
 		char yearTxt[32];
 		formatForMobile(year, yearTxt);

 		BW_INT total = getValueForFilter(filter->id, summary.total);
 		char totalTxt[32];
 		formatForMobile(total, totalTxt);
 		
 		tmp = strAppend(html, "<tr><td class='filter'>", filter->name, "</td><td class='amt'>", todayTxt, "</td><td class='amt'>", monthTxt, "</td><td class='amt'>", yearTxt, "</td><td class='amt'>", totalTxt, "</td></tr>", NULL);
 		free(html);
 		html = tmp;
 		filter = filter->next;	
 	}

	struct NameValuePair pair = {"summary", html, NULL};

    processFileRequest(fd, req, &pair);
    
    freeFilters(filters);
    free(html);
}
