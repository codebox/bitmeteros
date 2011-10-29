#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "bmws.h"

/*
Contains unit tests for the handleMonitor module.
*/

void testNoTsParam(void** state) {
 // The 'ts' parameter is required, so we should get an HTTP error if its missing
    struct Request req = {"GET", "/monitor", NULL, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    expect_string(mockWriteHeadersServerError, msg, "processMonitorRequest, ts parameter missing/invalid: %s");

    processMonitorRequest(0, &req);
	freeStmtList();
}

void testNoTgParam(void** state) {
 // The 'ts' parameter is required, so we should get an HTTP error if its missing
 	struct NameValuePair tsParam = {"ts", "120", NULL};
    struct Request req = {"GET", "/monitor", &tsParam, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    expect_string(mockWriteHeadersServerError, msg, "processMonitorRequest, fl parameter missing");

    processMonitorRequest(0, &req);
	freeStmtList();
}

void testMonitorParamsOkOneFilter(void** state) {
    struct NameValuePair tsParam = {"ts", "120", NULL};
    struct NameValuePair flParam = {"fl", "1", &tsParam};
    struct Request req = {"GET", "/monitor", &flParam, NULL};
    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
 // Expect to get 3 of these back only, the first lies in the future and the fifth is just outside the 120 second limit specified by 'ts'
    emptyDb();
    addDbRow(makeTs("2009-11-08 10:00:01"), 1,  1, 1);
    addDbRow(makeTs("2009-11-08 10:00:00"), 1,  2, 1);
    addDbRow(makeTs("2009-11-08 10:00:00"), 1,  2, 2);
    addDbRow(makeTs("2009-11-08 10:00:00"), 1,  2, 3);
    addDbRow(makeTs("2009-11-08 09:59:00"), 1,  4, 1);
    addDbRow(makeTs("2009-11-08 09:58:00"), 1,  8, 1);
    addDbRow(makeTs("2009-11-08 09:58:00"), 1,  8, 2);
    addDbRow(makeTs("2009-11-08 09:57:59"), 1, 16, 1);
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);
	expect_string(mockWriteText, txt, "{\"serverTime\" : 1257674400, \"data\" : ");

    expect_value(mockWriteDataToJsonTs, ts, 0);
    expect_value(mockWriteDataToJsonDr, dr, 1);
    expect_value(mockWriteDataToJsonTg, fl, 1);
    expect_value(mockWriteDataToJsonVl, vl, 2);

    expect_value(mockWriteDataToJsonTs, ts, 60);
    expect_value(mockWriteDataToJsonDr, dr, 1);
    expect_value(mockWriteDataToJsonTg, fl, 1);
    expect_value(mockWriteDataToJsonVl, vl, 4);

    expect_value(mockWriteDataToJsonTs, ts, 120);
    expect_value(mockWriteDataToJsonDr, dr, 1);
    expect_value(mockWriteDataToJsonTg, fl, 1);
    expect_value(mockWriteDataToJsonVl, vl, 8);

	expect_string(mockWriteText, txt, "}");
	
    processMonitorRequest(0, &req);
    
    freeStmtList();
}

void testMonitorParamsOkMultiFilter(void** state) {
    struct NameValuePair tsParam = {"ts", "120", NULL};
    struct NameValuePair flParam = {"fl", "1,3", &tsParam};
    struct Request req = {"GET", "/monitor", &flParam, NULL};
    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
 // Expect to get 3 of these back only, the first lies in the future and the fifth is just outside the 120 second limit specified by 'ts'
    emptyDb();
    addDbRow(makeTs("2009-11-08 10:00:01"), 1,  1, 1);
    addDbRow(makeTs("2009-11-08 10:00:00"), 1,  2, 1);
    addDbRow(makeTs("2009-11-08 10:00:00"), 1,  2, 2);
    addDbRow(makeTs("2009-11-08 10:00:00"), 1,  2, 3);
    addDbRow(makeTs("2009-11-08 09:59:00"), 1,  4, 1);
    addDbRow(makeTs("2009-11-08 09:58:00"), 1,  8, 1);
    addDbRow(makeTs("2009-11-08 09:58:00"), 1,  8, 2);
    addDbRow(makeTs("2009-11-08 09:57:59"), 1, 16, 1);
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);
	expect_string(mockWriteText, txt, "{\"serverTime\" : 1257674400, \"data\" : ");

    expect_value(mockWriteDataToJsonTs, ts, 0);
    expect_value(mockWriteDataToJsonDr, dr, 1);
    expect_value(mockWriteDataToJsonTg, fl, 1);
    expect_value(mockWriteDataToJsonVl, vl, 2);

    expect_value(mockWriteDataToJsonTs, ts, 60);
    expect_value(mockWriteDataToJsonDr, dr, 1);
    expect_value(mockWriteDataToJsonTg, fl, 1);
    expect_value(mockWriteDataToJsonVl, vl, 4);

    expect_value(mockWriteDataToJsonTs, ts, 120);
    expect_value(mockWriteDataToJsonDr, dr, 1);
    expect_value(mockWriteDataToJsonTg, fl, 1);
    expect_value(mockWriteDataToJsonVl, vl, 8);

    expect_value(mockWriteDataToJsonTs, ts, 0);
    expect_value(mockWriteDataToJsonDr, dr, 1);
    expect_value(mockWriteDataToJsonTg, fl, 3);
    expect_value(mockWriteDataToJsonVl, vl, 2);

	expect_string(mockWriteText, txt, "}");
	
    processMonitorRequest(0, &req);
    
    freeStmtList();
}

void testBuildFilterPairs(void** state){
	addFilterRow(1, "filter 1", "f1", "x1", NULL);
	addFilterRow(2, "filter 2", "f2", "x2", "host");
	addFilterRow(3, "filter 3", "f3", "x3", NULL);
	addFilterRow(4, "filter 4", "f4", "x4", NULL);
	
	struct NameValuePair* pairs;
	
	pairs = buildFilterPairs(NULL);
	
	struct NameValuePair* pair = pairs;
	assert_string_equal(pair->name, "f1");
	assert_string_equal(pair->value, "0.00 B ");

	pair = pair->next;	
	assert_string_equal(pair->name, "f2");
	assert_string_equal(pair->value, "0.00 B ");

	pair = pair->next;	
	assert_string_equal(pair->name, "f3");
	assert_string_equal(pair->value, "0.00 B ");
	
	pair = pair->next;	
	assert_string_equal(pair->name, "f4");
	assert_string_equal(pair->value, "0.00 B ");         
	
	assert_true(pair->next == NULL);  

	struct Data data1 = {1000, 1, 10, 1, NULL};
	struct Data data2 = {1001, 1, 20, 2, &data1};
	struct Data data3 = {1002, 1, 40, 3, &data2};
	struct Data data4 = {1002, 1,  0, 4, &data3}; // value=0, this filter should still be included in result
	struct Data data5 = {1002, 1, 10, 9, &data4}; // unknown filter, this should be skipped
	struct Data data6 = {1003, 1, 80, 1, &data5};
	
	freeNameValuePairs(pairs);
	
	pairs = buildFilterPairs(&data6);
	
	pair = pairs;
	assert_string_equal(pair->name, "f1");
	assert_string_equal(pair->value, "90.00 B ");
    
	pair = pair->next;	
	assert_string_equal(pair->name, "f2");
	assert_string_equal(pair->value, "20.00 B ");
    
	pair = pair->next;	
	assert_string_equal(pair->name, "f3");
	assert_string_equal(pair->value, "40.00 B ");
    
	pair = pair->next;	
	assert_string_equal(pair->name, "f4");
	assert_string_equal(pair->value, "0.00 B ");

	assert_true(pair->next == NULL);

	freeNameValuePairs(pairs);
	
	freeStmtList();
}

void testMakeHtmlFromData(void** state){
	char* html;
	
	html = makeHtmlFromData(NULL);
	assert_string_equal("<table id='monitor'></table>", html);
	free(html);
	
	struct NameValuePair pair1 = {"f3", "300 B ", NULL};
	struct NameValuePair pair2 = {"f2", "200 B ", &pair1};
	struct NameValuePair pair3 = {"f1", "100 B ", &pair2};
	
	html = makeHtmlFromData(&pair3);
	assert_string_equal("<table id='monitor'>" 
		"<tr><td class='filter'>f1</td><td class='amt'>100 B /s</td></tr>"
		"<tr><td class='filter'>f2</td><td class='amt'>200 B /s</td></tr>"
		"<tr><td class='filter'>f3</td><td class='amt'>300 B /s</td></tr>"
		"</table>"
		, html);
	free(html);
}