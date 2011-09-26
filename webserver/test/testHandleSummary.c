#include <stdio.h>
#include "common.h"
#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "client.h"
#include "bmws.h"

/*
Contains unit tests for the handleSummary module.
*/

static void _writeHeadersOk(SOCKET fd, char* contentType, int endHeaders){
	check_expected(contentType);
	check_expected(endHeaders);
}
static void _writeText(SOCKET fd, char* txt){
	check_expected(txt);
}
static void _writeNumValueToJson(SOCKET fd, char* key, BW_INT value){
	check_expected(key);
	check_expected(value);
}

void setupTestForHandleSummary(void** state){
	setupTestDb(state);
	
 	struct HandleSummaryCalls calls = {&_writeHeadersOk, &_writeText, &_writeNumValueToJson};
	mockHandleSummaryCalls = calls;
}

void tearDownTestForHandleSummary(void** state){
	tearDownTestDb(state);
}

void testHandleSummary(void** state) {
	addFilterRow(1, "Filter 1", "f1", "port 1", NULL);
	addFilterRow(2, "Filter 2", "f2", "port 2", "host1");
	addFilterRow(3, "Filter 3", "f3", "port 3", "otherhost");
		
    addDbRow(makeTsUtc("2008-11-02 12:00:00"), 3600, 1, 2); // Last year
    addDbRow(makeTsUtc("2008-11-02 12:00:00"), 3600, 1, 3); // Last year
    addDbRow(makeTsUtc("2009-10-01 12:00:00"), 3600, 2, 2); // Earlier this year
    addDbRow(makeTsUtc("2009-10-01 12:00:00"), 3600, 2, 1); // Earlier this year
    addDbRow(makeTsUtc("2009-11-04 12:00:00"), 3600, 4, 2); // Earlier this month
    addDbRow(makeTsUtc("2009-11-04 12:00:00"), 3600, 4, 1); // Earlier this month
    addDbRow(makeTsUtc("2009-11-08 01:00:00"), 3600, 8, 2); // Today
    addDbRow(makeTsUtc("2009-11-08 01:00:00"), 3600, 1, 2); // Today
    addDbRow(makeTsUtc("2009-11-08 01:00:00"), 3600, 8, 1); // Today

    struct Request req = {"GET", "/summary", NULL, NULL};

    time_t now = makeTsUtc("2009-11-08 10:00:00");
    setTime(now);

    expect_string(_writeHeadersOk, contentType, "application/json");
    expect_value(_writeHeadersOk, endHeaders, TRUE);
    expect_string(_writeText, txt, "{");
    expect_string(_writeText, txt, "\"today\": ");
	expect_string(_writeText, txt, "[");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 8);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 1);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, ",");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 9);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 2);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, "]");
	expect_string(_writeText, txt, ", ");
	
	expect_string(_writeText, txt, "\"month\": ");
	expect_string(_writeText, txt, "[");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 12);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 1);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, ",");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 13);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 2);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, "]");
	expect_string(_writeText, txt, ", ");

	expect_string(_writeText, txt, "\"year\": ");
	expect_string(_writeText, txt, "[");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 14);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 1);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, ",");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 15);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 2);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, "]");
	expect_string(_writeText, txt, ", ");

	expect_string(_writeText, txt, "\"total\": ");
	expect_string(_writeText, txt, "[");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 14);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 1);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, ",");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 16);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 2);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, ",");
	expect_string(_writeText, txt, "{");
	expect_string(_writeNumValueToJson, key, "vl");
	expect_value(_writeNumValueToJson, value, 1);
	expect_string(_writeText, txt, ",");
	expect_string(_writeNumValueToJson, key, "fl");
	expect_value(_writeNumValueToJson, value, 3);
	expect_string(_writeText, txt, "}");
	expect_string(_writeText, txt, "]");
	expect_string(_writeText, txt, ", ");
	
	expect_string(_writeText, txt, "\"hosts\": [");
	expect_string(_writeText, txt, "\"");
	expect_string(_writeText, txt, "host1");
	expect_string(_writeText, txt, "\"");
	expect_string(_writeText, txt, ", ");
	expect_string(_writeText, txt, "\"");
	expect_string(_writeText, txt, "otherhost");
	expect_string(_writeText, txt, "\"");
	expect_string(_writeText, txt, "]");
	
	expect_string(_writeText, txt, ", \"since\": ");
	expect_string(_writeText, txt, "1225627200");
	expect_string(_writeText, txt, "}");

    processSummaryRequest(0, &req);    
    freeStmtList();
}

 //getCalls().writeText(fd, "\"hosts\": [");
 //
 //       int i;
 //       for(i=0; i<summary.hostCount; i++){
 //           if (i>0){
 //               getCalls().writeText(fd, ", ");
 //           }
 //           getCalls().writeText(fd, "\"");
 //           getCalls().writeText(fd, summary.hostNames[i]);
 //           getCalls().writeText(fd, "\"");
 //       }
 //
 //       getCalls().writeText(fd, "]");
        
//        ", \"hosts\": null, \"since\": 1225627200}"


/*
getCalls().writeText(fd, "{");
	getCalls().writeTotal(fd, "today", summary.today);
	getCalls().writeText(fd, ", ");
	getCalls().writeTotal(fd, "month", summary.month);
	getCalls().writeText(fd, ", ");
	getCalls().writeTotal(fd, "year",  summary.year);
	getCalls().writeText(fd, ", ");
	getCalls().writeTotal(fd, "total", summary.total);
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
	*/