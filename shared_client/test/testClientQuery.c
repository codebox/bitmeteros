/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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

    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 08:00:00"), makeTsUtc("2009-03-03 11:00:00"), QUERY_GROUP_HOURS, NULL, NULL);
    CuAssertTrue(tc, data == NULL);
}

void testQueryNoDataInRange(CuTest *tc) {
 // Check that we behave correctly when no data matches the criteria
    emptyDb();

    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, "eth0", 1, 2, ""); // covers 07:00-08:00 so out of range
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, "eth0", 1, 2, ""); // covers 11:00-12:00 so out of range

    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 08:00:00"), makeTsUtc("2009-03-03 11:00:00"), QUERY_GROUP_HOURS, NULL, NULL);
    CuAssertTrue(tc, data == NULL);
}

void testQueryNoDataForHost(CuTest *tc) {
 // Check that we behave correctly when no data matches the criteria
    emptyDb();

    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, "eth0", 1, 2, "");
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, "eth0", 1, 2, "");

    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 00:00:00"), makeTsUtc("2009-03-04 00:00:00"), QUERY_GROUP_HOURS, "host1", NULL);
    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeHours(CuTest *tc) {
 // Check that results are correct when we group by hour
    emptyDb();

    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, "eth0", 1, 11, ""); // covers 07:00-08:00 so out of range
    addDbRow(makeTsUtc("2009-03-03 09:00:00"), 3600, "eth0", 2, 12, ""); // covers 08:00-09:00 so in range
    addDbRow(makeTsUtc("2009-03-03 11:00:00"), 3600, "eth0", 3, 13, ""); // covers 10:00-11:00 so in range
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, "eth0", 4, 14, ""); // covers 11:00-12:00 so out of range

    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 08:00:00"), makeTsUtc("2009-03-03 11:00:00"), QUERY_GROUP_HOURS, NULL, NULL);

    checkData(tc, data, makeTsUtc("2009-03-03 09:00:00"), 3600, NULL, 2, 12, NULL);
    data = data->next;

    checkData(tc, data, makeTsUtc("2009-03-03 11:00:00"), 3600, NULL, 3, 13, NULL);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeDays(CuTest *tc) {
 //  // Check that results are correct when we group by day
    emptyDb();

    addDbRow(makeTsUtc("2009-03-03 00:00:00"), 3600, "eth0", 1, 11,""); // out of range

    addDbRow(makeTsUtc("2009-03-03 01:00:00"), 3600, "eth0", 2, 12, "");
    addDbRow(makeTsUtc("2009-03-03 23:00:00"), 3600, "eth0", 3, 13, ""); // data for the 3rd
    addDbRow(makeTsUtc("2009-03-04 00:00:00"), 3600, "eth0", 4, 14, "");

    addDbRow(makeTsUtc("2009-03-04 01:00:00"), 3600, "eth0", 5, 15, "");
    addDbRow(makeTsUtc("2009-03-04 23:00:00"), 3600, "eth0", 6, 16, ""); // data for the 4th
    addDbRow(makeTsUtc("2009-03-05 00:00:00"), 3600, "eth0", 7, 17, "");

    addDbRow(makeTsUtc("2009-03-05 01:00:00"), 3600, "eth0", 8, 18, ""); // out of range

    struct Data* data = getQueryValues(makeTsUtc("2009-03-03 00:00:00"), makeTsUtc("2009-03-05 00:00:00"), QUERY_GROUP_DAYS, NULL, NULL);

    checkData(tc, data, makeTsUtc("2009-03-04 00:00:00"), 3600 * 24, NULL, 9, 39, NULL); // data for the 3rd, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTsUtc("2009-03-05 00:00:00"), 3600 * 24, NULL, 18, 48, NULL); // data for the 4th
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeMonths(CuTest *tc) {
 // Check that results are correct when we group by month
    emptyDb();

    addDbRow(makeTsUtc("2009-03-01 00:00:00"), 3600, "eth0", 1, 101, ""); // out of range

    addDbRow(makeTsUtc("2009-03-01 01:00:00"), 3600, "eth0", 2, 102, "");
    addDbRow(makeTsUtc("2009-03-30 23:00:00"), 3600, "eth0", 3, 103, ""); // data for March
    addDbRow(makeTsUtc("2009-04-01 00:00:00"), 3600, "eth0", 4, 104, "");

    addDbRow(makeTsUtc("2009-04-01 01:00:00"), 3600, "eth0", 5, 105, "");
    addDbRow(makeTsUtc("2009-04-30 23:00:00"), 3600, "eth0", 6, 106, ""); // data for April
    addDbRow(makeTsUtc("2009-05-01 00:00:00"), 3600, "eth0", 7, 107, "");

    addDbRow(makeTsUtc("2009-05-01 01:00:00"), 3600, "eth0", 8, 108, ""); // out of range

    struct Data* data = getQueryValues(makeTsUtc("2009-03-01 00:00:00"), makeTsUtc("2009-05-01 00:00:00"), QUERY_GROUP_MONTHS, NULL, NULL);

    checkData(tc, data, makeTsUtc("2009-04-01 00:00:00"), (31 * 24) * 3600, NULL, 9, 309, NULL); // data for March, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTsUtc("2009-05-01 00:00:00"), (30 * 24) * 3600, NULL, 18, 318, NULL); // data for April
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeYears(CuTest *tc) {
 // Check that results are correct when we group by year
    emptyDb();

    addDbRow(makeTsUtc("2007-01-01 00:00:00"), 3600, "eth0", 1, 101, ""); // out of range

    addDbRow(makeTsUtc("2007-01-01 01:00:00"), 3600, "eth0", 2, 102, "");
    addDbRow(makeTsUtc("2007-12-31 23:00:00"), 3600, "eth0", 3, 103, ""); // data for 2007
    addDbRow(makeTsUtc("2008-01-01 00:00:00"), 3600, "eth0", 4, 104, "");

    addDbRow(makeTsUtc("2008-01-01 01:00:00"), 3600, "eth0", 5, 105, "");
    addDbRow(makeTsUtc("2008-12-31 23:00:00"), 3600, "eth0", 6, 106, ""); // data for 2008
    addDbRow(makeTsUtc("2009-01-01 00:00:00"), 3600, "eth0", 7, 107, "");

    addDbRow(makeTsUtc("2009-01-01 01:00:00"), 3600, "eth0", 8, 108, ""); // out of range

    struct Data* data = getQueryValues(makeTsUtc("2007-01-01 00:00:00"), makeTsUtc("2009-01-01 00:00:00"), QUERY_GROUP_YEARS, NULL, NULL);

    checkData(tc, data, makeTsUtc("2008-01-01 00:00:00"), 365 * 3600 * 24, NULL, 9, 309, NULL); // data for 2007, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTsUtc("2009-01-01 00:00:00"), 366 * 3600 * 24, NULL, 18, 318, NULL); // data for 2008
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataNarrowValueRangeSingleResult(CuTest *tc) {
 // Check that results are correct when the range of values in the db is narrower than the range requested
    emptyDb();

    addDbRow(makeTsUtc("2008-02-01 01:00:00"), 3600, "eth0", 1, 101, "");
    addDbRow(makeTsUtc("2008-03-31 23:00:00"), 3600, "eth0", 2, 102, "");
    addDbRow(makeTsUtc("2008-04-01 00:00:00"), 3600, "eth0", 3, 103, "");

    struct Data* data = getQueryValues(makeTsUtc("2008-01-01 00:00:00"), makeTsUtc("2009-01-01 00:00:00"), QUERY_GROUP_YEARS, NULL, NULL);

    checkData(tc, data, makeTsUtc("2008-04-01 00:00:00"), ((29 + 31) * 24) * 3600, NULL, 6, 306, NULL);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataNarrowValueRangeMultiResults(CuTest *tc) {
 // Check that results are correct when the range of values in the db is narrower than the range requested
    emptyDb();

    addDbRow(makeTsUtc("2007-05-01 01:00:00"), 3600, "eth0", 1, 101, "");

    addDbRow(makeTsUtc("2008-02-01 01:00:00"), 3600, "eth0", 2, 102, "");
    addDbRow(makeTsUtc("2008-03-31 23:00:00"), 3600, "eth0", 3, 103, "");
    addDbRow(makeTsUtc("2008-04-01 00:00:00"), 3600, "eth0", 4, 104, "");

    addDbRow(makeTsUtc("2009-05-01 01:00:00"), 3600, "eth0", 5, 105, "");

    struct Data* data = getQueryValues(makeTsUtc("2006-01-01 00:00:00"), makeTsUtc("2011-01-01 00:00:00"), QUERY_GROUP_YEARS, NULL, NULL);

    checkData(tc, data, makeTsUtc("2008-01-01 00:00:00"), 245 * 24 * 60 * 60, NULL, 1, 101, NULL); 

    data = data->next;
    checkData(tc, data, makeTsUtc("2009-01-01 00:00:00"), 31622400, NULL, 9, 309, NULL);

    data = data->next;
    checkData(tc, data, makeTsUtc("2010-01-01 00:00:00"), 31536000, NULL, 5, 105, NULL);

    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryLargeQueryRange(CuTest *tc) {
 // Check that results are correct when we dont group, and just produce a total
    emptyDb();

    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, "eth0", 1, 11, "");
    addDbRow(makeTsUtc("2009-03-03 09:00:00"), 3600, "eth0", 2, 12, "");
    addDbRow(makeTsUtc("2009-03-03 11:00:00"), 3600, "eth0", 3, 13, "");
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, "eth0", 4, 14, "");

    struct Data* data = getQueryValues(makeTsUtc("2000-01-01 00:00:00"), makeTsUtc("2019-01-01 00:00:00"), QUERY_GROUP_TOTAL, NULL, NULL);

    checkData(tc, data, makeTsUtc("2009-03-03 12:00:00"), 18000, NULL, 10, 50, NULL);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryFilterByHost(CuTest *tc) {
 // Check that results are correct when we specify that only data for a single host should be returned
    emptyDb();

    addDbRow(makeTsUtc("2009-03-02 14:00:00"), 3600, "eth0", 256, 2560, "host1"); // out of range

    addDbRow(makeTsUtc("2009-03-03 08:00:00"), 3600, "eth0", 1,   10, "");
    addDbRow(makeTsUtc("2009-03-03 09:00:00"), 3600, "eth0", 2,   20, "host1");
    addDbRow(makeTsUtc("2009-03-03 11:00:00"), 3600, "eth0", 4,   40, "host2");
    addDbRow(makeTsUtc("2009-03-03 12:00:00"), 3600, "eth0", 8,   80, "");
    addDbRow(makeTsUtc("2009-03-03 13:00:00"), 3600, "eth0", 16, 160, "host1");
    addDbRow(makeTsUtc("2009-03-03 14:00:00"), 3600, "eth0", 32, 320, "host3");
    addDbRow(makeTsUtc("2009-03-03 14:00:00"), 3600, "eth0", 64, 640, "host1");

    addDbRow(makeTsUtc("2009-03-04 14:00:00"), 3600, "eth0", 128, 1280, "host1"); // out of range

    struct Data* data1 = getQueryValues(makeTsUtc("2009-03-03 00:00:00"), makeTsUtc("2009-03-04 00:00:00"), QUERY_GROUP_HOURS, "host1", NULL);

    checkData(tc, data1, makeTsUtc("2009-03-03 09:00:00"), 3600, NULL, 2, 20, NULL);
    data1 = data1->next;

    checkData(tc, data1, makeTsUtc("2009-03-03 13:00:00"), 3600, NULL, 16, 160, NULL);
    data1 = data1->next;

    checkData(tc, data1, makeTsUtc("2009-03-03 14:00:00"), 3600, NULL, 64, 640, NULL);
    data1 = data1->next;

    CuAssertTrue(tc, data1 == NULL);

    struct Data* data2 = getQueryValues(makeTsUtc("2009-03-03 00:00:00"), makeTsUtc("2009-03-04 00:00:00"), QUERY_GROUP_HOURS, "", NULL);

    checkData(tc, data2, makeTsUtc("2009-03-03 08:00:00"), 3600, NULL, 1, 10, NULL);
    data2 = data2->next;

    checkData(tc, data2, makeTsUtc("2009-03-03 12:00:00"), 3600, NULL, 8, 80, NULL);
    data2 = data2->next;

    CuAssertTrue(tc, data2 == NULL);
}

void testQueryFilterByHostAndAdapter(CuTest *tc) {
 // Check that results are correct when we specify that only data for a single host/adapter combination should be returned
    emptyDb();

    addDbRow(makeTsUtc("2009-03-01 00:00:00"), 3600, "eth0", 1, 101, "host1"); // out of range

    addDbRow(makeTsUtc("2009-03-01 01:00:00"), 3600, "eth0", 2, 102, "");
    addDbRow(makeTsUtc("2009-03-30 23:00:00"), 3600, "eth0", 3, 103, "");
    addDbRow(makeTsUtc("2009-04-01 00:00:00"), 3600, "eth0", 4, 104, "");

    addDbRow(makeTsUtc("2009-04-01 01:00:00"), 3600, "eth0", 5, 105, "");
    addDbRow(makeTsUtc("2009-04-30 23:00:00"), 3600, "eth0", 6, 106, "");
    addDbRow(makeTsUtc("2009-05-01 00:00:00"), 3600, "eth0", 7, 107, "");

    addDbRow(makeTsUtc("2009-05-01 01:00:00"), 3600, "eth0", 8, 108, "host1"); // out of range

    struct Data* data = getQueryValues(makeTsUtc("2009-03-01 00:00:00"), makeTsUtc("2009-05-01 00:00:00"), QUERY_GROUP_MONTHS, NULL, NULL);

    checkData(tc, data, makeTsUtc("2009-04-01 00:00:00"), 31 * 3600 * 24, NULL, 9, 309, NULL); // data for March, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTsUtc("2009-05-01 00:00:00"), 30 * 3600 * 24, NULL, 18, 318, NULL); // data for April
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

CuSuite* clientQueryGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testQueryEmptyDb);
    SUITE_ADD_TEST(suite, testQueryNoDataInRange);
    SUITE_ADD_TEST(suite, testQueryNoDataForHost);
    SUITE_ADD_TEST(suite, testQueryDataInRangeHours);
    SUITE_ADD_TEST(suite, testQueryDataInRangeDays);
    SUITE_ADD_TEST(suite, testQueryDataInRangeMonths);
    SUITE_ADD_TEST(suite, testQueryDataInRangeYears);
    SUITE_ADD_TEST(suite, testQueryLargeQueryRange);
    SUITE_ADD_TEST(suite, testQueryDataNarrowValueRangeSingleResult);
    SUITE_ADD_TEST(suite, testQueryDataNarrowValueRangeMultiResults);
    SUITE_ADD_TEST(suite, testQueryFilterByHost);
    
    return suite;
}
