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
/*
struct Summary{
	struct Data* today;
	struct Data* month;
	struct Data* year;
	struct Data* total;
	time_t tsMin;
	time_t tsMax;
	char** hostNames;
	int    hostCount;
};*/

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
void processMobileSummaryRequest(SOCKET fd, struct Request* req){
	struct Summary summary = getSummaryValues(NULL, NULL);
//TODO
 // Daily amounts
	//char dlDayTxt[32];
	//formatForMobile(summary.today->dl, dlDayTxt);
    //
	//char ulDayTxt[32];
	//formatForMobile(summary.today->ul, ulDayTxt);
    //
	//char cmDayTxt[32];
	//formatForMobile(summary.today->dl + summary.today->ul, cmDayTxt);
    //
 // //Monthly amounts
	//char dlMonthTxt[32];
	//formatForMobile(summary.month->dl, dlMonthTxt);
    //
	//char ulMonthTxt[32];
	//formatForMobile(summary.month->ul, ulMonthTxt);
    //
	//char cmMonthTxt[32];
	//formatForMobile(summary.month->dl + summary.month->ul, cmMonthTxt);
    //
 // //Yearly amounts
	//char dlYearTxt[32];
	//formatForMobile(summary.year->dl, dlYearTxt);
    //
	//char ulYearTxt[32];
	//formatForMobile(summary.year->ul, ulYearTxt);
    //
	//char cmYearTxt[32];
	//formatForMobile(summary.year->dl + summary.year->ul, cmYearTxt);
    //
 // //Total amounts
	//char dlTotalTxt[32];
	//formatForMobile(summary.total->dl, dlTotalTxt);
    //
	//char ulTotalTxt[32];
	//formatForMobile(summary.total->ul, ulTotalTxt);
    //
	//char cmTotalTxt[32];
	//formatForMobile(summary.total->dl + summary.total->ul, cmTotalTxt);
    //
    //
	//struct NameValuePair pair1  = {"dlDay", dlDayTxt,   NULL};
	//struct NameValuePair pair2  = {"ulDay", ulDayTxt,   &pair1};
	//struct NameValuePair pair3  = {"cmDay", cmDayTxt,   &pair2};
	//struct NameValuePair pair4  = {"dlMonth", dlMonthTxt, &pair3};
	//struct NameValuePair pair5  = {"ulMonth", ulMonthTxt, &pair4};
	//struct NameValuePair pair6  = {"cmMonth", cmMonthTxt, &pair5};
	//struct NameValuePair pair7  = {"dlYear", dlYearTxt,  &pair6};
	//struct NameValuePair pair8  = {"ulYear", ulYearTxt,  &pair7};
	//struct NameValuePair pair9  = {"cmYear", cmYearTxt,  &pair8};
	//struct NameValuePair pair10 = {"dlTotal", dlTotalTxt, &pair9};
	//struct NameValuePair pair11 = {"ulTotal", ulTotalTxt, &pair10};
	//struct NameValuePair pair12 = {"cmTotal", cmTotalTxt, &pair11};

    //processFileRequest(fd, req, &pair12);
}
