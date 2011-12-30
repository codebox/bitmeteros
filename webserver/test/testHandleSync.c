#include <stdio.h>
#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "client.h"
#include "bmws.h"

/*
Contains unit tests for the handleSync module.
*/

void testSyncNoTsParam(void** state) {
 // The 'ts' parameter is required, so we should get an HTTP error if its missing
    struct Request req = {"GET", "/sync", NULL, NULL};
    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    expect_string(mockWriteHeadersServerError, msg, "processSyncRequest ts param missing/invalid: %s");
    
    processSyncRequest(0, &req);
}

void testSyncTsParamOk(void** state) {
    char ts[20];
    sprintf(ts, "%d", (int)makeTs("2009-11-01 10:00:00"));
    struct NameValuePair param = {"ts", ts, NULL};
    struct Request req = {"GET", "/sync", &param, NULL};
    
    time_t now = makeTs("2009-11-01 10:00:00");
    setTime(now);
    
    emptyDb();
    addFilterRow(1, "Filter 1", "f1", "port 1", NULL);
    addFilterRow(2, "Filter 2", "f2", "port 2", "host1");
    addFilterRow(3, "Filter 3", "f3", "port 3", NULL);
    
    addDbRow(makeTsUtc("2009-10-31 10:00:00"), 1,  1, 1); // too early
    addDbRow(makeTsUtc("2009-11-01 10:00:00"), 1,  2, 2);
    addDbRow(makeTsUtc("2009-11-01 10:00:01"), 1,  4, 3); // this one
    addDbRow(makeTsUtc("2009-11-01 10:00:01"), 1,  8, 1); // this one
    addDbRow(makeTsUtc("2009-11-02 00:00:00"), 1, 16, 2); 
    addDbRow(makeTsUtc("2009-11-02 00:00:00"), 1, 32, 2);
    addDbRow(makeTsUtc("2009-11-02 01:00:00"), 1, 64, 3); // this one
    addDbRow(makeTsUtc("2009-11-08 10:00:00"), 1,128, 1); // this one
    
    expect_string(mockWriteHeadersOk, contentType, "application/vnd.codebox.bitmeter-sync");
    expect_value(mockWriteHeadersOk, endHeaders, 1);
    
    expect_value(mockCheckFilterValues, id, 1);
    expect_string(mockCheckFilterValues, name, "f1");
    expect_value(mockCheckFilterValues, id, 3);
    expect_string(mockCheckFilterValues, name, "f3");
    
    expect_value(mockCheckDataValues, filterId, 3);
    expect_value(mockCheckDataValues, value, 4);
    expect_value(mockCheckDataValues, filterId, 1);
    expect_value(mockCheckDataValues, value, 8);
    expect_value(mockCheckDataValues, filterId, 3);
    expect_value(mockCheckDataValues, value, 64);
    expect_value(mockCheckDataValues, filterId, 1);
    expect_value(mockCheckDataValues, value, 128);
    
    processSyncRequest(0, &req);
    freeStmtList();
}

