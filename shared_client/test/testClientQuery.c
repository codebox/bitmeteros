#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "client.h"

/*
Contains unit tests for the clientQuery module.
*/

void testQueryEmptyDb(void** status) {
 // Check that we behave correctly when the data table is empty
    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 08:00:00"), makeTsUtc("2009-03-03 11:00:00"), QUERY_GROUP_HOURS, FILTER);
    assert_true(data == NULL);
    freeStmtList();
}

void testQueryNoDataInRange(void** status) {
 // Check that we behave correctly when no data matches the criteria
    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, 1, FILTER); // covers 07:00-08:00 so out of range
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, 1, FILTER); // covers 11:00-12:00 so out of range

    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 08:00:00"), makeTsUtc("2009-03-03 11:00:00"), QUERY_GROUP_HOURS, FILTER);
    assert_true(data == NULL);
    freeStmtList();
}

void testQueryNoDataForHost(void** status) {
 // Check that we behave correctly when no data matches the criteria
    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, 1, FILTER2);
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, 1, FILTER2);

    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 00:00:00"), makeTsUtc("2009-03-04 00:00:00"), QUERY_GROUP_HOURS, FILTER);
    assert_true(data == NULL);
    freeStmtList();
}

void testQueryDataInRangeHours(void** status) {
 // Check that results are correct when we group by hour
    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, 1, FILTER);  // covers 07:00-08:00 so out of range
    addDbRow(makeTsUtc("2009-03-03 09:00:00"), 3600, 2, FILTER);  // covers 08:00-09:00 so in range
    addDbRow(makeTsUtc("2009-03-03 09:00:00"), 3600, 2, FILTER2); // wrong filter
    addDbRow(makeTsUtc("2009-03-03 11:00:00"), 3600, 3, FILTER);  // covers 10:00-11:00 so in range
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, 4, FILTER);  // covers 11:00-12:00 so out of range

	struct Data* first;
    struct Data* data = first = getQueryValues(makeTsUtc("2009-03-03 08:00:00"), makeTsUtc("2009-03-03 11:00:00"), QUERY_GROUP_HOURS, FILTER);

    checkData(data, makeTsUtc("2009-03-03 09:00:00"), 3600, 2, FILTER);
    data = data->next;

    checkData(data, makeTsUtc("2009-03-03 11:00:00"), 3600, 3, FILTER);
    data = data->next;

    assert_true(data == NULL);
    freeData(first);
    freeStmtList();
}

void testQueryDataInRangeDays(void** status) {
 // Check that results are correct when we group by day
    addDbRow(makeTsUtc("2009-03-03 00:00:00"), 3600, 1, FILTER);  // out of range

    addDbRow(makeTsUtc("2009-03-03 01:00:00"), 3600, 2, FILTER);
    addDbRow(makeTsUtc("2009-03-03 23:00:00"), 3600, 3, FILTER);  // data for the 3rd
    addDbRow(makeTsUtc("2009-03-04 00:00:00"), 3600, 4, FILTER);
    addDbRow(makeTsUtc("2009-03-04 00:00:00"), 3600, 4, FILTER2); // wrong filter

    addDbRow(makeTsUtc("2009-03-04 01:00:00"), 3600, 5, FILTER);
    addDbRow(makeTsUtc("2009-03-04 23:00:00"), 3600, 6, FILTER);  // data for the 4th
    addDbRow(makeTsUtc("2009-03-05 00:00:00"), 3600, 7, FILTER);

    addDbRow(makeTsUtc("2009-03-05 01:00:00"), 3600, 8, FILTER);  // out of range

	struct Data* first;
    struct Data* data = first = getQueryValues(makeTsUtc("2009-03-03 00:00:00"), makeTsUtc("2009-03-05 00:00:00"), QUERY_GROUP_DAYS, FILTER);

    checkData(data, makeTsUtc("2009-03-04 00:00:00"), 3600 * 24, 9, FILTER); // data for the 3rd, remember ts is the END of the interval
    data = data->next;

    checkData(data, makeTsUtc("2009-03-05 00:00:00"), 3600 * 24, 18, FILTER); // data for the 4th
    data = data->next;

    assert_true(data == NULL);
    freeData(first);
    freeStmtList();
}

void testQueryDataInRangeMonths(void** status) {
 // Check that results are correct when we group by month
    addDbRow(makeTsUtc("2009-03-01 00:00:00"), 3600, 1, FILTER);  // out of range - this covers that last hour of February

    addDbRow(makeTsUtc("2009-03-01 01:00:00"), 3600, 2, FILTER);
    addDbRow(makeTsUtc("2009-03-30 23:00:00"), 3600, 3, FILTER);  // data for March
    addDbRow(makeTsUtc("2009-04-01 00:00:00"), 3600, 4, FILTER);

    addDbRow(makeTsUtc("2009-04-01 01:00:00"), 3600, 5, FILTER2); // wrong filter
    addDbRow(makeTsUtc("2009-04-01 01:00:00"), 3600, 5, FILTER);
    addDbRow(makeTsUtc("2009-04-30 23:00:00"), 3600, 6, FILTER);  // data for April
    addDbRow(makeTsUtc("2009-05-01 00:00:00"), 3600, 7, FILTER);

    addDbRow(makeTsUtc("2009-05-01 01:00:00"), 3600, 8, FILTER);  // out of range

 // Query for March and April...
	struct Data* first;
    struct Data* data = first = getQueryValues(makeTsUtc("2009-03-01 00:00:00"), makeTsUtc("2009-05-01 00:00:00"), QUERY_GROUP_MONTHS, FILTER);

    checkData(data, makeTsUtc("2009-04-01 00:00:00"), (31 * 24) * 3600, 9, FILTER); // data for March, remember ts is the END of the interval
    data = data->next;

    checkData(data, makeTsUtc("2009-05-01 00:00:00"), (30 * 24) * 3600, 18, FILTER); // data for April
    data = data->next;

    assert_true(data == NULL);
    freeData(first);
    freeStmtList();
}

void testQueryDataInRangeYears(void** status) {
 // Check that results are correct when we group by year
    addDbRow(makeTsUtc("2007-01-01 00:00:00"), 3600, 1, FILTER);  // out of range

    addDbRow(makeTsUtc("2007-01-01 01:00:00"), 3600, 2, FILTER);
    addDbRow(makeTsUtc("2007-12-31 23:00:00"), 3600, 3, FILTER);  // data for 2007
    addDbRow(makeTsUtc("2008-01-01 00:00:00"), 3600, 4, FILTER);

    addDbRow(makeTsUtc("2008-01-01 01:00:00"), 3600, 5, FILTER);
    addDbRow(makeTsUtc("2008-12-31 23:00:00"), 3600, 6, FILTER);  // data for 2008
    addDbRow(makeTsUtc("2009-01-01 00:00:00"), 3600, 7, FILTER);

    addDbRow(makeTsUtc("2009-01-01 01:00:00"), 3600, 8, FILTER);  // out of range
    addDbRow(makeTsUtc("2009-01-01 01:00:00"), 3600, 8, FILTER2); // out of range and wrong filter

 // Query for 2007 and 2008
	struct Data* first;
    struct Data* data = first = getQueryValues(makeTsUtc("2007-01-01 00:00:00"), makeTsUtc("2009-01-01 00:00:00"), QUERY_GROUP_YEARS, FILTER);

    checkData(data, makeTsUtc("2008-01-01 00:00:00"), 365 * 3600 * 24, 9, FILTER); // data for 2007, remember ts is the END of the interval
    data = data->next;

    checkData(data, makeTsUtc("2009-01-01 00:00:00"), 366 * 3600 * 24, 18, FILTER); // data for 2008
    data = data->next;

    assert_true(data == NULL);
    freeData(first);
    freeStmtList();
}

void testQueryDataNarrowValueRangeSingleResult(void** status) {
 // Check that results are correct when the range of values in the db is narrower than the range requested
    addDbRow(makeTsUtc("2008-02-01 01:00:00"), 3600, 1, FILTER);
    addDbRow(makeTsUtc("2008-03-31 23:00:00"), 3600, 2, FILTER);
    addDbRow(makeTsUtc("2008-03-31 23:00:00"), 3600, 2, FILTER2);
    addDbRow(makeTsUtc("2008-04-01 00:00:00"), 3600, 3, FILTER);

	struct Data* first;
    struct Data* data = first = getQueryValues(makeTsUtc("2008-01-01 00:00:00"), makeTsUtc("2009-01-01 00:00:00"), QUERY_GROUP_YEARS, FILTER);

    checkData(data, makeTsUtc("2008-04-01 00:00:00"), ((29 + 31) * 24) * 3600, 6, FILTER);
    data = data->next;

    assert_true(data == NULL);
    freeData(first);
    freeStmtList();
}

void testQueryDataNarrowValueRangeMultiResults(void** status) {
 // Check that results are correct when the range of values in the db is narrower than the range requested
    addDbRow(makeTsUtc("2007-05-01 01:00:00"), 3600, 1, FILTER);
                                                        
    addDbRow(makeTsUtc("2008-02-01 01:00:00"), 3600, 2, FILTER);
    addDbRow(makeTsUtc("2008-03-31 23:00:00"), 3600, 3, FILTER);
    addDbRow(makeTsUtc("2008-04-01 00:00:00"), 3600, 4, FILTER);
    addDbRow(makeTsUtc("2008-04-01 00:00:00"), 3600, 4, FILTER2);
                                                        
    addDbRow(makeTsUtc("2009-05-01 01:00:00"), 3600, 5, FILTER);

	struct Data* first;
    struct Data* data = first = getQueryValues(makeTsUtc("2006-01-01 00:00:00"), makeTsUtc("2011-01-01 00:00:00"), QUERY_GROUP_YEARS, FILTER);

    checkData(data, makeTsUtc("2008-01-01 00:00:00"), 245 * 24 * 60 * 60, 1, FILTER); 

    data = data->next;
    checkData(data, makeTsUtc("2009-01-01 00:00:00"), 31622400, 9, FILTER);

    data = data->next;
    checkData(data, makeTsUtc("2010-01-01 00:00:00"), 31536000, 5, FILTER);

    data = data->next;

    assert_true(data == NULL);
    freeData(first);
    freeStmtList();
}

void testQueryLargeQueryRange(void** status) {
 // Check that results are correct when we dont group, and just produce a total
    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, 1, FILTER);
    addDbRow(makeTsUtc("2009-03-03 09:00:00"), 3600, 2, FILTER);
    addDbRow(makeTsUtc("2009-03-03 11:00:00"), 3600, 3, FILTER);
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, 4, FILTER);
    addDbRow(makeTsUtc("2009-03-03 13:00:00"), 3600, 5, FILTER2);

	struct Data* first;
    struct Data* data = first = getQueryValues(makeTsUtc("2000-01-01 00:00:00"), makeTsUtc("2019-01-01 00:00:00"), QUERY_GROUP_TOTAL, FILTER);

    checkData(data, makeTsUtc("2009-03-03 12:00:00"), 18000, 10, FILTER);
    data = data->next;

    assert_true(data == NULL);
    freeData(first);
    freeStmtList();
}
