#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"
#include "common.h"
#include "client.h"

/*
Contains unit tests for the clientUtil module.
*/

static void checkTsBounds(struct ValueBounds* tsBounds, time_t min, time_t max);

void testCalcTsBoundsEmptyDb(void** state) {
 // Check that we behave correctly when the data table is empty
    assert_true(calcTsBounds(FILTER)  == NULL);
    assert_true(calcTsBounds(FILTER2) == NULL);
    freeStmtList();
}

void testCalcTsBoundsNoMatches(void** state) {
 // Check that we behave correctly when the data table contains data but no host/adapter matches are found
    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, 1, FILTER);
    addDbRow(makeTs("2009-05-01 02:00:00"), 3600, 2, FILTER);

    assert_true(calcTsBounds(FILTER2) == NULL);
    freeStmtList();
}

void testCalcTsBoundsWithMatches(void** state) {
 // Check that we behave correctly when the data table contains data that matches the host/adapter
    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, 1, FILTER);
    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, 1, FILTER2);
    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, 1, FILTER3);
    addDbRow(makeTs("2009-05-01 02:00:00"), 3600, 1, FILTER);
    addDbRow(makeTs("2009-05-01 02:00:00"), 3600, 1, FILTER2);
    addDbRow(makeTs("2009-05-01 03:00:00"), 3600, 1, FILTER); 

	struct ValueBounds* tsBounds;
	
	tsBounds = calcTsBounds(FILTER);
    checkTsBounds(tsBounds,  makeTs("2009-05-01 01:00:00"), makeTs("2009-05-01 03:00:00"));
    free(tsBounds);
    
    tsBounds = calcTsBounds(FILTER2);
    checkTsBounds(tsBounds, makeTs("2009-05-01 01:00:00"), makeTs("2009-05-01 02:00:00"));
    free(tsBounds);
    
    tsBounds = calcTsBounds(FILTER3);
    checkTsBounds(tsBounds, makeTs("2009-05-01 01:00:00"), makeTs("2009-05-01 01:00:00"));
    free(tsBounds);
    freeStmtList();
}

static void checkTsBounds(struct ValueBounds* tsBounds, time_t min, time_t max){
    assert_int_equal(min, tsBounds->min);
    assert_int_equal(max, tsBounds->max);
}

void testGetValueForFilterIdNull(void** state) {
	assert_int_equal(0, getValueForFilterId(NULL, FILTER));
}

void testGetValueForFilterIdNoMatch(void** state) {
	struct Data* data1 = allocData(1, 1, 1, FILTER2);
	struct Data* data2 = allocData(1, 1, 1, FILTER3);
	struct Data* data3 = allocData(1, 1, 1, FILTER2);
	data1->next = data2;
	data2->next = data3;
	
	assert_int_equal(0, getValueForFilterId(data1, FILTER));
	freeData(data1);
}

void testGetValueForFilterIdWithMatch(void** state) {
	struct Data data3 = {1, 1, 1, FILTER2, NULL};
	struct Data data2 = {1, 1, 2, FILTER,  &data3};
	struct Data data1 = {1, 1, 3, FILTER2, &data2};
	
	assert_int_equal(2, getValueForFilterId(&data1, FILTER));
}

void testCalcMaxValueEmptyDb(void** state) {
	struct Data* data = calcMaxValue();
	assert_int_equal(0, data->vl);
	freeData(data);
	freeStmtList();
}

void testCalcMaxValueWithData(void** state) {
    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, 1,   FILTER);
    addDbRow(makeTs("2009-05-01 02:00:00"), 3600, 100, FILTER);
    addDbRow(makeTs("2009-05-01 03:00:00"), 3600, 10,  FILTER);
	
	struct Data* data = calcMaxValue();
	assert_int_equal(100, data->vl);
	freeData(data);
	freeStmtList();
}

void testFormatAmountByUnits(void** state) {
	char txt[64];
	
	formatAmountByUnits(0, &txt, UNITS_BYTES);
	assert_string_equal("0", txt);
	
	formatAmountByUnits(99999, &txt, UNITS_BYTES);
	assert_string_equal("99999", txt);

	formatAmountByUnits(0, &txt, UNITS_ABBREV);
	assert_string_equal("0.00 B ", txt);

	formatAmountByUnits(1024, &txt, UNITS_ABBREV);
	assert_string_equal("1.00 kB", txt);

	formatAmountByUnits(0, &txt, UNITS_FULL);
	assert_string_equal("0.00 bytes", txt);

	formatAmountByUnits(1024, &txt, UNITS_FULL);
	assert_string_equal("1.00 kilobytes", txt);
}