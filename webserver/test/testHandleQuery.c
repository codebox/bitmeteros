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
Contains unit tests for the handleQuery module.
*/

static void _writeHeadersServerError(SOCKET fd, char* msg, ...){
	check_expected(msg);
}
static void _writeHeadersOk(SOCKET fd, char* contentType, int endHeaders){
	check_expected(contentType);
	check_expected(endHeaders);
}
static void _writeHeader(SOCKET fd, char* name, char* value){
	check_expected(name);
	check_expected(value);
}
static void _writeEndOfHeaders(SOCKET fd){
	check_expected(fd);
}
static void _writeDataToJsonTs(time_t ts){
	check_expected(ts);
}
static void _writeDataToJsonVl(BW_INT vl){
	check_expected(vl);
}
static void _writeDataToJsonDr(int dr){
	check_expected(dr);
}
static void _writeDataToJsonTg(int fl){
	check_expected(fl);
}
static void _writeDataToJson(SOCKET fd, struct Data* data){
	while(data != NULL){
		_writeDataToJsonTs(data->ts);
		_writeDataToJsonVl(data->vl);
		_writeDataToJsonDr(data->dr);
		_writeDataToJsonTg(data->fl);
		data = data->next;
	}
}
static void _writeText(SOCKET fd, char* txt){
	check_expected(txt);
}

void setupTestForHandleQuery(void** state){
	setupTestDb(state);
	addFilterRow(FILTER, "filter desc", "filter", "expr", NULL);
	struct HandleQueryCalls calls = {&_writeHeadersServerError, &_writeHeadersOk, &_writeHeader,
			&_writeEndOfHeaders, &_writeDataToJson, &_writeText};
	mockHandleQueryCalls = calls;
};

void tearDownTestForHandleQuery(void** state){
	tearDownTestDb(state);
}

void testMissingParam(void** state) {
 // The 3 parameters are required, so we should get an HTTP error if they are missing
    struct Request req = {"GET", "/query", NULL, NULL};
    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
	expect_string(_writeHeadersServerError, msg, "processQueryRequest, param bad/missing from=%s, to=%s, group=%s, fl=%d");
    
    processQueryRequest(0, &req);
    
	freeStmtList();
}

void testParamsOkOneFilter(void** state) {
    struct NameValuePair fromParam  = {"from", "1257120000", NULL};       // 2009-11-02
    struct NameValuePair toParam    = {"to",   "1257292800", &fromParam}; // 2009-11-04
    struct NameValuePair groupParam = {"group", "5", &toParam};           // Total
    struct NameValuePair flParam    = {"fl", "1", &groupParam};  
    struct Request req = {"GET", "/query", &flParam, NULL};
    
 // The query range covers the second, third and fourth rows only
    emptyDb();
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600,  1, 1);
    addDbRow(makeTs("2009-11-02 12:00:00"), 3600,  2, 1); // Match
    addDbRow(makeTs("2009-11-03 12:00:00"), 3600,  4, 1); // Match
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600,  8, 1); // Match
    addDbRow(makeTs("2009-11-05 12:00:00"), 3600, 16, 1);
    
    expect_string(_writeHeadersOk, contentType, "application/json");
    expect_value(_writeHeadersOk, endHeaders, TRUE);
    
 // The 'ts' value = 2009-11-05 00:00:00, ie the end of the date range covered by the query
 // The 'dr' value = 3 * 24 * 3600, ie the number of seconds in 3 days
    expect_value(_writeDataToJsonTs, ts, 1257379200);
    expect_value(_writeDataToJsonDr, dr, 259200);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 14);
    processQueryRequest(0, &req);
                                    
	freeStmtList();                                    	
}

void testParamsOkMultiFilter(void** state) {
    struct NameValuePair fromParam  = {"from", "1257120000", NULL};       // 2009-11-02
    struct NameValuePair toParam    = {"to",   "1257292800", &fromParam}; // 2009-11-04
    struct NameValuePair groupParam = {"group", "5", &toParam};           // Total
    struct NameValuePair flParam    = {"fl", "1,3,4", &groupParam};  
    struct Request req = {"GET", "/query", &flParam, NULL};
    
 // The query range covers the second, third and fourth rows only
    emptyDb();
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600,  1, 1);
    addDbRow(makeTs("2009-11-02 12:00:00"), 3600,  2, 1); // Match
    addDbRow(makeTs("2009-11-03 12:00:00"), 3600,  4, 3); // Match
    addDbRow(makeTs("2009-11-03 12:00:00"), 3600,  4, 1); // Match
    addDbRow(makeTs("2009-11-03 12:00:00"), 3600,  4, 2); // No match - bad filter
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600,  8, 4); // Match
    addDbRow(makeTs("2009-11-05 12:00:00"), 3600, 16, 1);
    
    expect_string(_writeHeadersOk, contentType, "application/json");
    expect_value(_writeHeadersOk, endHeaders, TRUE);
    
 // The 'ts' value = 2009-11-05 00:00:00, ie the end of the date range covered by the query
 // The 'dr' value = 3 * 24 * 3600, ie the number of seconds in 3 days
    expect_value(_writeDataToJsonTs, ts, 1257379200);
    expect_value(_writeDataToJsonDr, dr, 259200);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 6);

    expect_value(_writeDataToJsonTs, ts, 1257249600);
    expect_value(_writeDataToJsonDr, dr, 3600);
    expect_value(_writeDataToJsonTg, fl, 3);
    expect_value(_writeDataToJsonVl, vl, 4);

    expect_value(_writeDataToJsonTs, ts, 1257336000);
    expect_value(_writeDataToJsonDr, dr, 3600);
    expect_value(_writeDataToJsonTg, fl, 4);
    expect_value(_writeDataToJsonVl, vl, 8);

    processQueryRequest(0, &req);
                                    
	freeStmtList();                                    	
}

void testGroupByDay(void** state) {
    struct NameValuePair fromParam  = {"from", "0", NULL};
    struct NameValuePair toParam    = {"to",   "1258281927", &fromParam};
    struct NameValuePair groupParam = {"group", "2", &toParam};
    struct NameValuePair flParam    = {"fl", "1", &groupParam};  
    struct Request req = {"GET", "/query", &flParam, NULL};
    
    emptyDb();
    addDbRow(makeTs("2009-11-01 10:00:00"), 3600,  1, 1);
    addDbRow(makeTs("2009-11-01 11:00:00"), 3600,  2, 1);
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600,  4, 1);
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600,  4, 2); // wrong filter
    addDbRow(makeTs("2009-11-02 09:00:00"), 3600,  8, 1);
    addDbRow(makeTs("2009-11-02 23:00:00"), 3600, 16, 1);
    addDbRow(makeTs("2009-11-02 23:00:00"), 3600, 32, 3); // wrong filter
    
    expect_string(_writeHeadersOk, contentType, "application/json");
    expect_value(_writeHeadersOk, endHeaders, TRUE);
    
    expect_value(_writeDataToJsonTs, ts, 1257120000);
    expect_value(_writeDataToJsonDr, dr, 54000);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 7);

    expect_value(_writeDataToJsonTs, ts, 1257206400);
    expect_value(_writeDataToJsonDr, dr, 86400);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 24);

    processQueryRequest(0, &req);
                                    
	freeStmtList();                                    	
}

void testParamsOkReversed(void** state) {
    struct NameValuePair fromParam  = {"from", "1257292800", NULL};       // 2009-11-04
    struct NameValuePair toParam    = {"to",   "1257206400", &fromParam}; // 2009-11-03
    struct NameValuePair groupParam = {"group", "2", &toParam};           // Days
    struct NameValuePair flParam    = {"fl", "1", &groupParam}; 
    struct Request req = {"GET", "/query", &flParam, NULL};
    
 // The query range covers the third and fourth rows only
    emptyDb();
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600,  1, 1);
    addDbRow(makeTs("2009-11-02 12:00:00"), 3600,  2, 1);
    addDbRow(makeTs("2009-11-03 12:00:00"), 3600,  4, 1); // Match
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600,  8, 1); // Match
    addDbRow(makeTs("2009-11-05 12:00:00"), 3600, 16, 1);
    
    expect_string(_writeHeadersOk, contentType, "application/json");
    expect_value(_writeHeadersOk, endHeaders, TRUE);
    
 // The 'ts' values = 2009-11-04 00:00:00 and 2009-11-05 00:00:00, ie the ends of the 2 days
 // The 'dr' value = 24 * 3600, ie the number of seconds in a day
    expect_value(_writeDataToJsonTs, ts, 1257292800);
    expect_value(_writeDataToJsonDr, dr, 86400);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 4);

    expect_value(_writeDataToJsonTs, ts, 1257379200);
    expect_value(_writeDataToJsonDr, dr, 86400);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 8);

    processQueryRequest(0, &req);
    
	freeStmtList();
}

void testGroupByDayCsv(void** state) {
    struct NameValuePair fromParam  = {"from", "0", NULL};
    struct NameValuePair toParam    = {"to",   "1258281927", &fromParam};
    struct NameValuePair groupParam = {"group", "2", &toParam};
    struct NameValuePair csvParam   = {"csv", "1", &groupParam};
    struct NameValuePair flParam    = {"fl", "1", &csvParam};
    struct Request req = {"GET", "/query", &flParam, NULL};
    
    emptyDb();
    addDbRow(makeTs("2009-11-01 10:00:00"), 3600,  1, FILTER);
    addDbRow(makeTs("2009-11-01 11:00:00"), 3600,  2, FILTER);
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600,  4, FILTER);
    addDbRow(makeTs("2009-11-02 09:00:00"), 3600,  8, FILTER);
    addDbRow(makeTs("2009-11-02 23:00:00"), 3600, 16, FILTER);
    
    expect_string(_writeHeadersOk, contentType, "text/csv");
    expect_value(_writeHeadersOk, endHeaders, FALSE);
    expect_string(_writeHeader, name, "Content-Disposition");
    expect_string(_writeHeader, value, "attachment;filename=bitmeterOsQuery.csv");
	expect_value(_writeEndOfHeaders, fd, 0);
    expect_string(_writeText, txt, "2009-11-01 09:00:00,7,filter\n");
    expect_string(_writeText, txt, "2009-11-02 00:00:00,24,filter\n");
    
    processQueryRequest(0, &req);
    
	freeStmtList();
}
