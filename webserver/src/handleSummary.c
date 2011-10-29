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

void processSummaryRequest(SOCKET fd, struct Request* req){
    WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);

	struct Summary summary = getSummaryValues();

	WRITE_TEXT(fd, "{");
	writeTotal(fd, "today", summary.today);
	WRITE_TEXT(fd, ", ");
	writeTotal(fd, "month", summary.month);
	WRITE_TEXT(fd, ", ");
	writeTotal(fd, "year",  summary.year);
	WRITE_TEXT(fd, ", ");
	writeTotal(fd, "total", summary.total);
	WRITE_TEXT(fd, ", ");

	if (summary.hostNames != NULL){
        WRITE_TEXT(fd, "\"hosts\": [");

        int i;
        for(i=0; i<summary.hostCount; i++){
            if (i>0){
                WRITE_TEXT(fd, ", ");
            }
            WRITE_TEXT(fd, "\"");
            WRITE_TEXT(fd, summary.hostNames[i]);
            WRITE_TEXT(fd, "\"");
        }

        WRITE_TEXT(fd, "]");
	} else {
	    WRITE_TEXT(fd, "\"hosts\": null");
	}

	WRITE_TEXT(fd, ", \"since\": ");

	char sinceTs[12];
    unsigned long since = (unsigned long) summary.tsMin;
	sprintf(sinceTs, "%lu", since);
	WRITE_TEXT(fd, sinceTs);
	WRITE_TEXT(fd, "}");

	freeSummary(&summary);
}

static void writeTotal(SOCKET fd, char* totalName, struct Data* data){
 // Helper function, writes out one of the summary totals in the correct JSON format
 	char prefix[32];
 	sprintf(prefix, "\"%s\": ", totalName);
	WRITE_TEXT(fd, prefix);
	
	WRITE_TEXT(fd, "[");
	int firstItem = TRUE;
	while(data != NULL){
		if (!firstItem){
			WRITE_TEXT(fd, ",");	
		}

		WRITE_TEXT(fd, "{");
		WRITE_NUM_VALUE_TO_JSON(fd, "vl", data->vl);
		WRITE_TEXT(fd, ",");
		WRITE_NUM_VALUE_TO_JSON(fd, "fl", data->fl);
		WRITE_TEXT(fd, "}");
		firstItem = FALSE;
		
		data = data->next;	
	}
	
	WRITE_TEXT(fd, "]");
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
