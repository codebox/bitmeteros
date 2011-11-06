#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "client.h"

/*
Contains unit tests for the clientSync module.
*/

void testSyncEmptyDb(void** state) {
 // Check that we behave correctly when the data table is empty
    assert_true(getSyncValues(1) == NULL);
    freeStmtList();
}

void testSyncNoMatchingData(void** state) {
 // Set up filters for other hosts
    addFilterRow(FILTER,  "1", "1", "", NULL);
    addFilterRow(FILTER2, "2", "2", "", NULL);
    addFilterRow(FILTER3, "3", "3", "", "host");
    
 // Check that we behave correctly when the data table contains rows but none meet our criterion
    addDbRow(0, 1, 123, FILTER);
    addDbRow(1, 1, 123, FILTER2);
    addDbRow(3, 1, 123, FILTER3);

    assert_true(getSyncValues(1) == NULL);
    freeStmtList();
}

void testSyncDataOnAndAfterTs(void** state) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and have differing timestamps */
    addDbRow(9,  1, 1, FILTER);
    addDbRow(10, 1, 1, FILTER2);
    addDbRow(10, 1, 2, FILTER);
    addDbRow(10, 1, 2, FILTER3);
    addDbRow(11, 1, 3, FILTER);
    addDbRow(11, 1, 4, FILTER3);

 // Set up filters for other hosts
    addFilterRow(FILTER,  "1", "1", "", NULL);
    addFilterRow(FILTER2, "2", "2", "", NULL);
    addFilterRow(FILTER3, "3", "3", "", "host");

    struct Data* first;
    struct Data* data = first = getSyncValues(9);
    checkData(data, 10, 1, 1, FILTER2);

    data = data->next;
    checkData(data, 10, 1, 2, FILTER);

    data = data->next;
    checkData(data, 11, 1, 3, FILTER);

    assert_true(data->next == NULL);
    freeData(first);
    freeStmtList();
}
