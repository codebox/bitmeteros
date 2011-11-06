#include <stdlib.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "client.h"
#include "test.h"

/*
Contains unit tests for the clientDump utility.
*/

static void onDumpRow(int ignored, struct Data* data);
struct Data* dumpResult;

void testClientDumpEmptyDb(void **state) {
 // Check that we behave correctly if the data table is empty
    dumpResult = NULL;
    getDumpValues(0, &onDumpRow);
    assert_true(dumpResult == NULL);
    freeStmtList();
}

void testClientDumpOneEntry(void **state) {
 // Check the results if we only have single row in the table
    dumpResult = NULL;

    addDbRow(1234, 1, 2, 3);
    getDumpValues(0, &onDumpRow);
    checkData(dumpResult, 1234, 1, 2, 3);

    assert_true(dumpResult->next == NULL);
    freeData(dumpResult);
    freeStmtList();    
}

void testClientDumpMultipleEntries(void **state) {
 // Check the results if we have multiple rows in the table
    dumpResult = NULL;

    addDbRow(1233, 1, 4, 7);
    addDbRow(1234, 2, 5, 8);
    addDbRow(1235, 3, 6, 9);

    getDumpValues(0, &onDumpRow);
    struct Data* first = dumpResult;
    
    checkData(dumpResult, 1235, 3, 6, 9);

    dumpResult = dumpResult->next;
    checkData(dumpResult, 1234, 2, 5, 8);

    dumpResult = dumpResult->next;
    checkData(dumpResult, 1233, 1, 4, 7);

    dumpResult = dumpResult->next;
    assert_true(dumpResult == NULL);
    freeData(first);
    freeStmtList();    
}

static void onDumpRow(int ignored, struct Data* data) {
 // Helper callback function used by tests to record each Data struct that is returned
    appendData(&dumpResult, data);
}

