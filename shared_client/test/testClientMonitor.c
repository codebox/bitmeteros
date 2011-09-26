#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "client.h"

/*
Contains unit tests for the clientMonitor module.
*/

void testMonitorEmptyDb(void** state) {
 // Check that we behave correctly when the data table is empty
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();

    assert_true(getMonitorValues(now - 1, FILTER) == NULL);
    freeStmtList();
}

void testMonitorNoDataAfterTs(void** state) {
 // Check that we behave correctly when the data table contains rows but none meet our criterion
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1, 123, FILTER);
    
    assert_true(getMonitorValues(now - 1, FILTER) == NULL);
    freeStmtList();
}

void testMonitorDataOnTs(void** state) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and they all have the same timestamp */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, 1, FILTER);
    addDbRow(now - 1, 1, 2, FILTER);
    addDbRow(now - 1, 1, 3, FILTER);
    
    struct Data* data = getMonitorValues(now - 1, FILTER);
    checkData(data, now-1, 1, 6, FILTER); // We group data by ts, so expect just 1 result
    assert_true(data->next == NULL);
    freeData(data);
    freeStmtList();
}   

void testMonitorDataOnAndAfterTs(void** state) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and have differing timestamps */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, 1, FILTER);
    addDbRow(now - 1, 1, 1, FILTER);
    addDbRow(now - 1, 1, 1, FILTER);
    
    addDbRow(now - 2, 1, 5, FILTER);
    addDbRow(now - 2, 1, 5, FILTER);
    addDbRow(now - 2, 1, 5, FILTER);
    
    struct Data* first;
    struct Data* data = first = getMonitorValues(now - 2, FILTER);
    checkData(data, now-1, 1, 3, FILTER);
    
    data = data->next;
    checkData(data, now-2, 1, 15, FILTER);
    
    assert_true(data->next == NULL);
    freeData(first);
    freeStmtList();
}

void testMonitorDataOnAndLongAfterTs(void** state) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and have differing non-consecutive timestamps */
    time_t now  = makeTs("2009-01-01 10:00:00");
    time_t then = makeTs("2008-10-10 11:00:00");
    
    setTime(now);
    emptyDb();
    addDbRow(now, 1, 1, FILTER);
    addDbRow(now, 1, 1, FILTER);
    addDbRow(now, 1, 1, FILTER);
    
    addDbRow(then + 3600, 3600, 5, FILTER);
    addDbRow(then,        3600, 6, FILTER);
    addDbRow(then - 3600, 3600, 7, FILTER);
                                
	struct Data* first;
    struct Data* data = first = getMonitorValues(then, FILTER);
    checkData(data, now, 1, 3, FILTER);
    
    data = data->next;
    checkData(data, then + 3600, 3600, 5, FILTER);
    
    data = data->next;
    checkData(data, then, 3600, 6, FILTER);
    
    assert_true(data->next == NULL);
    freeData(first);
    freeStmtList();
}

void testMonitorDataForFilter(void** state) {
 /* Check that data selection by filter works correctly. */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1,   1, FILTER); // too old
    addDbRow(now - 1, 1,   2, FILTER2);
    addDbRow(now - 1, 1,   4, FILTER); // want this
    addDbRow(now - 1, 1,   8, FILTER); // want this
    addDbRow(now - 1, 1,  16, FILTER); // want this
    addDbRow(now - 1, 1,  32, FILTER2);
    addDbRow(now + 1, 1,  64, FILTER); // want this
    addDbRow(now + 1, 1, 128, FILTER2);
    
    struct Data* first;
    struct Data* data = first = getMonitorValues(now - 1, FILTER);
    checkData(data, now + 1, 1, 64, FILTER);
    data = data->next;
    checkData(data, now - 1, 1, 28, FILTER);
    
    assert_true(data->next == NULL);
    freeData(first);
    freeStmtList();
}

void testMonitorDataForAllFilters(void** state) {
 /* Check that data selection by filter works correctly. */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1,   1, FILTER); // too old
    addDbRow(now - 1, 1,   2, FILTER2);
    addDbRow(now - 1, 1,   4, FILTER);
    addDbRow(now - 1, 1,   8, FILTER);
    addDbRow(now - 1, 1,  16, FILTER);
    addDbRow(now - 1, 1,  32, FILTER2);
    addDbRow(now + 1, 1,  64, FILTER);
    addDbRow(now + 1, 1, 128, FILTER2);
    
    struct Data* first;
    struct Data* data = first = getMonitorValues(now - 1, 0);
    checkData(data, now + 1, 1, 64, FILTER);
    data = data->next;
    checkData(data, now + 1, 1, 128, FILTER2);
    data = data->next;
    checkData(data, now - 1, 1, 28, FILTER);
    data = data->next;
    checkData(data, now - 1, 1, 34, FILTER2);
    
    assert_true(data->next == NULL);
    freeData(first);
    freeStmtList();
}
