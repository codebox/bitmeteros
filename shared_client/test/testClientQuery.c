/*
 * BitMeterOS v0.3.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

#include "test.h"
#include "common.h"
#include "client.h"
#include "CuTest.h"

/*
Contains unit tests for the clientQuery module.
*/

void testQueryEmptyDb(CuTest *tc) {
 // Check that we behave correctly when the data table is empty
    emptyDb();

    struct Data* data = getQueryValues(makeTs("2009-03-03 08:00:00"), makeTs("2009-03-03 11:00:00"), QUERY_GROUP_HOURS);
    CuAssertTrue(tc, data == NULL);
}

void testQueryNoDataInRange(CuTest *tc) {
 // Check that we behave correctly when no data matches the criteria
    emptyDb();

    addDbRow(makeTs("2009-03-03 08:00:00"), 3600, "eth0", 1, 2, NULL); // covers 07:00-08:00 so out of range
    addDbRow(makeTs("2009-03-03 12:00:00"), 3600, "eth0", 1, 2, NULL); // covers 11:00-12:00 so out of range

    struct Data* data = getQueryValues(makeTs("2009-03-03 08:00:00"), makeTs("2009-03-03 11:00:00"), QUERY_GROUP_HOURS);
    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeHours(CuTest *tc) {
 // Check that results are correct when we group by hour
    emptyDb();

    addDbRow(makeTs("2009-03-03 08:00:00"), 3600, "eth0", 1, 11, NULL); // covers 07:00-08:00 so out of range
    addDbRow(makeTs("2009-03-03 09:00:00"), 3600, "eth0", 2, 12, NULL); // covers 08:00-09:00 so in range
    addDbRow(makeTs("2009-03-03 11:00:00"), 3600, "eth0", 3, 13, NULL); // covers 10:00-11:00 so in range
    addDbRow(makeTs("2009-03-03 12:00:00"), 3600, "eth0", 4, 14, NULL); // covers 11:00-12:00 so out of range

    struct Data* data = getQueryValues(makeTs("2009-03-03 08:00:00"), makeTs("2009-03-03 11:00:00"), QUERY_GROUP_HOURS);

    checkData(tc, data, makeTs("2009-03-03 09:00:00"), 3600, NULL, 2, 12, NULL);
    data = data->next;

    checkData(tc, data, makeTs("2009-03-03 11:00:00"), 3600, NULL, 3, 13, NULL);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeDays(CuTest *tc) {
 //  // Check that results are correct when we group by day
    emptyDb();

    addDbRow(makeTs("2009-03-03 00:00:00"), 3600, "eth0", 1, 11, NULL); // out of range

    addDbRow(makeTs("2009-03-03 01:00:00"), 3600, "eth0", 2, 12, NULL);
    addDbRow(makeTs("2009-03-03 23:00:00"), 3600, "eth0", 3, 13, NULL); // data for the 3rd
    addDbRow(makeTs("2009-03-04 00:00:00"), 3600, "eth0", 4, 14, NULL);

    addDbRow(makeTs("2009-03-04 01:00:00"), 3600, "eth0", 5, 15, NULL);
    addDbRow(makeTs("2009-03-04 23:00:00"), 3600, "eth0", 6, 16, NULL); // data for the 4th
    addDbRow(makeTs("2009-03-05 00:00:00"), 3600, "eth0", 7, 17, NULL);

    addDbRow(makeTs("2009-03-05 01:00:00"), 3600, "eth0", 8, 18, NULL); // out of range

    struct Data* data = getQueryValues(makeTs("2009-03-03 00:00:00"), makeTs("2009-03-05 00:00:00"), QUERY_GROUP_DAYS);

    checkData(tc, data, makeTs("2009-03-04 00:00:00"), 3600 * 24, NULL, 9, 39, NULL); // data for the 3rd, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTs("2009-03-05 00:00:00"), 3600 * 24, NULL, 18, 48, NULL); // data for the 4th
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeMonths(CuTest *tc) {
 // Check that results are correct when we group by month
    emptyDb();

    addDbRow(makeTs("2009-03-01 00:00:00"), 3600, "eth0", 1, 101, NULL); // out of range

    addDbRow(makeTs("2009-03-01 01:00:00"), 3600, "eth0", 2, 102, NULL);
    addDbRow(makeTs("2009-03-30 23:00:00"), 3600, "eth0", 3, 103, NULL); // data for March
    addDbRow(makeTs("2009-04-01 00:00:00"), 3600, "eth0", 4, 104, NULL);

    addDbRow(makeTs("2009-04-01 01:00:00"), 3600, "eth0", 5, 105, NULL);
    addDbRow(makeTs("2009-04-30 23:00:00"), 3600, "eth0", 6, 106, NULL); // data for April
    addDbRow(makeTs("2009-05-01 00:00:00"), 3600, "eth0", 7, 107, NULL);

    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, "eth0", 8, 108, NULL); // out of range

    struct Data* data = getQueryValues(makeTs("2009-03-01 00:00:00"), makeTs("2009-05-01 00:00:00"), QUERY_GROUP_MONTHS);

    checkData(tc, data, makeTs("2009-04-01 00:00:00"), 31 * 3600 * 24, NULL, 9, 309, NULL); // data for March, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTs("2009-05-01 00:00:00"), 30 * 3600 * 24, NULL, 18, 318, NULL); // data for April
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeYears(CuTest *tc) {
 // Check that results are correct when we group by year
    emptyDb();

    addDbRow(makeTs("2007-01-01 00:00:00"), 3600, "eth0", 1, 101, NULL); // out of range

    addDbRow(makeTs("2007-01-01 01:00:00"), 3600, "eth0", 2, 102, NULL);
    addDbRow(makeTs("2007-12-31 23:00:00"), 3600, "eth0", 3, 103, NULL); // data for 2007
    addDbRow(makeTs("2008-01-01 00:00:00"), 3600, "eth0", 4, 104, NULL);

    addDbRow(makeTs("2008-01-01 01:00:00"), 3600, "eth0", 5, 105, NULL);
    addDbRow(makeTs("2008-12-31 23:00:00"), 3600, "eth0", 6, 106, NULL); // data for 2008
    addDbRow(makeTs("2009-01-01 00:00:00"), 3600, "eth0", 7, 107, NULL);

    addDbRow(makeTs("2009-01-01 01:00:00"), 3600, "eth0", 8, 108, NULL); // out of range

    struct Data* data = getQueryValues(makeTs("2007-01-01 00:00:00"), makeTs("2009-01-01 00:00:00"), QUERY_GROUP_YEARS);

    checkData(tc, data, makeTs("2008-01-01 00:00:00"), 365 * 3600 * 24, NULL, 9, 309, NULL); // data for 2007, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTs("2009-01-01 00:00:00"), 366 * 3600 * 24, NULL, 18, 318, NULL); // data for 2008
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataNarrowValueRangeSingleResult(CuTest *tc) {
 // Check that results are correct when the range of values in the db is narrower than the range requested
    emptyDb();

    addDbRow(makeTs("2008-02-01 01:00:00"), 3600, "eth0", 1, 101, NULL);
    addDbRow(makeTs("2008-03-31 23:00:00"), 3600, "eth0", 2, 102, NULL);
    addDbRow(makeTs("2008-04-01 00:00:00"), 3600, "eth0", 3, 103, NULL);

    struct Data* data = getQueryValues(makeTs("2008-01-01 00:00:00"), makeTs("2009-01-01 00:00:00"), QUERY_GROUP_YEARS);

    checkData(tc, data, makeTs("2008-04-01 00:00:00"), 5180401, NULL, 6, 306, NULL);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataNarrowValueRangeMultiResults(CuTest *tc) {
 // Check that results are correct when the range of values in the db is narrower than the range requested
    emptyDb();

    addDbRow(makeTs("2007-05-01 01:00:00"), 3600, "eth0", 1, 101, NULL);

    addDbRow(makeTs("2008-02-01 01:00:00"), 3600, "eth0", 2, 102, NULL);
    addDbRow(makeTs("2008-03-31 23:00:00"), 3600, "eth0", 3, 103, NULL);
    addDbRow(makeTs("2008-04-01 00:00:00"), 3600, "eth0", 4, 104, NULL);

    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, "eth0", 5, 105, NULL);

    struct Data* data = getQueryValues(makeTs("2006-01-01 00:00:00"), makeTs("2011-01-01 00:00:00"), QUERY_GROUP_YEARS);

    checkData(tc, data, makeTs("2008-01-01 00:00:00"), 21164401, NULL, 1, 101, NULL);

    data = data->next;
    checkData(tc, data, makeTs("2009-01-01 00:00:00"), 31622400, NULL, 9, 309, NULL);

    data = data->next;
    checkData(tc, data, makeTs("2010-01-01 00:00:00"), 31536000, NULL, 5, 105, NULL);

    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryLargeQueryRange(CuTest *tc) {
 // Check that results are correct when we dont group, and just produce a total
    emptyDb();

    addDbRow(makeTs("2009-03-03 08:00:00"), 3600, "eth0", 1, 11, NULL);
    addDbRow(makeTs("2009-03-03 09:00:00"), 3600, "eth0", 2, 12, NULL);
    addDbRow(makeTs("2009-03-03 11:00:00"), 3600, "eth0", 3, 13, NULL);
    addDbRow(makeTs("2009-03-03 12:00:00"), 3600, "eth0", 4, 14, NULL);

    struct Data* data = getQueryValues(makeTs("2000-01-01 00:00:00"), makeTs("2019-01-01 00:00:00"), QUERY_GROUP_TOTAL);

    checkData(tc, data, makeTs("2009-03-03 12:00:00"), 14401, NULL, 10, 50, NULL);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

CuSuite* clientQueryGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testQueryEmptyDb);
    SUITE_ADD_TEST(suite, testQueryNoDataInRange);
    SUITE_ADD_TEST(suite, testQueryDataInRangeHours);
    SUITE_ADD_TEST(suite, testQueryDataInRangeDays);
    SUITE_ADD_TEST(suite, testQueryDataInRangeMonths);
    SUITE_ADD_TEST(suite, testQueryDataInRangeYears);
    SUITE_ADD_TEST(suite, testQueryLargeQueryRange);
    SUITE_ADD_TEST(suite, testQueryDataNarrowValueRangeSingleResult);
    SUITE_ADD_TEST(suite, testQueryDataNarrowValueRangeMultiResults);
    return suite;
}
