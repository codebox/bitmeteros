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

    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteText, txt, "\"today\": ");
	expect_string(mockWriteText, txt, "[");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 8);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 1);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 9);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 2);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, "]");
	expect_string(mockWriteText, txt, ", ");
	
	expect_string(mockWriteText, txt, "\"month\": ");
	expect_string(mockWriteText, txt, "[");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 12);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 1);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 13);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 2);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, "]");
	expect_string(mockWriteText, txt, ", ");

	expect_string(mockWriteText, txt, "\"year\": ");
	expect_string(mockWriteText, txt, "[");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 14);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 1);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 15);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 2);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, "]");
	expect_string(mockWriteText, txt, ", ");

	expect_string(mockWriteText, txt, "\"total\": ");
	expect_string(mockWriteText, txt, "[");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 14);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 1);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 16);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 2);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteText, txt, "{");
	expect_string(mockWriteNumValueToJson, key, "vl");
	expect_value(mockWriteNumValueToJson, value, 1);
	expect_string(mockWriteText, txt, ",");
	expect_string(mockWriteNumValueToJson, key, "fl");
	expect_value(mockWriteNumValueToJson, value, 3);
	expect_string(mockWriteText, txt, "}");
	expect_string(mockWriteText, txt, "]");
	expect_string(mockWriteText, txt, ", ");
	
	expect_string(mockWriteText, txt, "\"hosts\": [");
	expect_string(mockWriteText, txt, "\"");
	expect_string(mockWriteText, txt, "host1");
	expect_string(mockWriteText, txt, "\"");
	expect_string(mockWriteText, txt, ", ");
	expect_string(mockWriteText, txt, "\"");
	expect_string(mockWriteText, txt, "otherhost");
	expect_string(mockWriteText, txt, "\"");
	expect_string(mockWriteText, txt, "]");
	
	expect_string(mockWriteText, txt, ", \"since\": ");
	expect_string(mockWriteText, txt, "1225627200");
	expect_string(mockWriteText, txt, "}");

    processSummaryRequest(0, &req);    
    freeStmtList();
}
