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
static void _writeHeadersServerError(SOCKET fd, char* msg, ...){
	check_expected(msg);
}
static void _writeHeadersOk(SOCKET fd, char* contentType, int endHeaders){
	check_expected(contentType);
	check_expected(endHeaders);
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

void setupTestForHandleMonitor(void** state){
	setupTestDb(state);
	struct HandleMonitorCalls calls = {&_writeHeadersServerError, &_writeHeadersOk, &_writeDataToJson, &_writeText};
	mockHandleMonitorCalls = calls;
};

void tearDownTestForHandleMonitor(void** state){
	tearDownTestDb(state);
}

void testNoTsParam(void** state) {
 // The 'ts' parameter is required, so we should get an HTTP error if its missing
    struct Request req = {"GET", "/monitor", NULL, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    expect_string(_writeHeadersServerError, msg, "processMonitorRequest, ts parameter missing/invalid: %s");

    processMonitorRequest(0, &req);
	freeStmtList();
}

void testNoTgParam(void** state) {
 // The 'ts' parameter is required, so we should get an HTTP error if its missing
 	struct NameValuePair tsParam = {"ts", "120", NULL};
    struct Request req = {"GET", "/monitor", &tsParam, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    expect_string(_writeHeadersServerError, msg, "processMonitorRequest, fl parameter missing");

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
    
    expect_string(_writeHeadersOk, contentType, "application/json");
    expect_value(_writeHeadersOk, endHeaders, TRUE);
	expect_string(_writeText, txt, "{\"serverTime\" : 1257674400, \"data\" : ");

    expect_value(_writeDataToJsonTs, ts, 0);
    expect_value(_writeDataToJsonDr, dr, 1);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 2);

    expect_value(_writeDataToJsonTs, ts, 60);
    expect_value(_writeDataToJsonDr, dr, 1);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 4);

    expect_value(_writeDataToJsonTs, ts, 120);
    expect_value(_writeDataToJsonDr, dr, 1);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 8);

	expect_string(_writeText, txt, "}");
	
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
    
    expect_string(_writeHeadersOk, contentType, "application/json");
    expect_value(_writeHeadersOk, endHeaders, TRUE);
	expect_string(_writeText, txt, "{\"serverTime\" : 1257674400, \"data\" : ");

    expect_value(_writeDataToJsonTs, ts, 0);
    expect_value(_writeDataToJsonDr, dr, 1);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 2);

    expect_value(_writeDataToJsonTs, ts, 60);
    expect_value(_writeDataToJsonDr, dr, 1);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 4);

    expect_value(_writeDataToJsonTs, ts, 120);
    expect_value(_writeDataToJsonDr, dr, 1);
    expect_value(_writeDataToJsonTg, fl, 1);
    expect_value(_writeDataToJsonVl, vl, 8);

    expect_value(_writeDataToJsonTs, ts, 0);
    expect_value(_writeDataToJsonDr, dr, 1);
    expect_value(_writeDataToJsonTg, fl, 3);
    expect_value(_writeDataToJsonVl, vl, 2);

	expect_string(_writeText, txt, "}");
	
    processMonitorRequest(0, &req);
    
    freeStmtList();
}