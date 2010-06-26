/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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
Contains unit tests for the clientSummary module.
*/

static void checkSummary(CuTest *, struct Summary, time_t, time_t, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, int, char**);

void testSummaryEmptyDb(CuTest *tc) {
 // Check that we behave correctly when the data table is empty
    emptyDb();
    checkSummary(tc, getSummaryValues(NULL, NULL), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
}

void testSummaryOneEntry(CuTest *tc) {
 // Check that we behave correctly when the data table contains only 1 row
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 123, 456, "");
    checkSummary(tc, getSummaryValues(NULL, NULL), now, now, 123, 456, 123, 456, 123, 456, 123, 456, 0, NULL);
}

void testSummaryTwoEntriesSameTime(CuTest *tc) {
 // Check that we behave correctly when the data table contains 2 entries with the same timestamp
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 100, 400, "");
    addDbRow(now, 1, "eth1", 23, 56, "");
    checkSummary(tc, getSummaryValues(NULL, NULL), now, now, 123, 456, 123, 456, 123, 456, 123, 456, 0, NULL);
}

void testSummaryTwoEntriesDifferentTimes(CuTest *tc) {
 // Check that we behave correctly when the data table contains 2 entries with different timestamps
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 100, 400, "");
    addDbRow(now - 2, 1, "eth1", 23, 56, "");
    checkSummary(tc, getSummaryValues(NULL, NULL), now - 2, now - 1, 123, 456, 123, 456, 123, 456, 123, 456, 0, NULL);
}

void testSummaryEntriesSpanningDayBoundary(CuTest *tc) {
 // Check that results are correct when our data spans a day boundary
    time_t now = makeTs("2009-01-02 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10, ""); //today
    addDbRow(now - 1, 1, "eth1", 2, 11, ""); //today
    addDbRow(now - 2, 1, "eth0", 4, 12, ""); //yesterday
    checkSummary(tc, getSummaryValues(NULL, NULL), now - 2, now, 3, 21, 7, 33, 7, 33, 7, 33, 0, NULL);
}

void testSummaryEntriesSpanningMonthBoundary(CuTest *tc) {
 // Check that results are correct when our data spans a month boundary
    time_t now = makeTs("2009-02-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10, ""); //today
    addDbRow(now - 1, 1, "eth1", 2, 11, ""); //today
    addDbRow(now - 2, 1, "eth0", 4, 12, ""); //yesterday and last month
    checkSummary(tc, getSummaryValues(NULL, NULL), now - 2, now, 3, 21, 3, 21, 7, 33, 7, 33, 0, NULL);
}

void testSummaryEntriesSpanningYearBoundary(CuTest *tc) {
 // Check that results are correct when our data spans a year boundary
    time_t now = makeTs("2009-01-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10, ""); //today
    addDbRow(now - 1, 1, "eth1", 2, 11, ""); //today
    addDbRow(now - 2, 1, "eth0", 4, 12, ""); //yesterday and last year
    checkSummary(tc, getSummaryValues(NULL, NULL), now - 2, now, 3, 21, 3, 21, 3, 21, 7, 33, 0, NULL);
}

void testSummaryMultipleEntries(CuTest *tc) {
 // Check that results are correct when we have multiple entries spread over a large date range
    time_t now = makeTs("2009-03-02 10:00:00");
    setTime(now);
    emptyDb();

 // Entries for today
    addDbRow(makeTs("2009-03-02 10:00:00"), 1,    "eth1", 1, 10, "");
    addDbRow(makeTs("2009-03-02 09:59:59"), 1,    "eth0", 1, 10, "host1");
    addDbRow(makeTs("2009-03-02 08:00:00"), 60,   "eth0", 1, 10, "");
    addDbRow(makeTs("2009-03-02 07:59:00"), 60,   "eth1", 1, 10, "");
    addDbRow(makeTs("2009-03-02 07:58:00"), 60,   "eth0", 1, 10, "host1");
    addDbRow(makeTs("2009-03-02 01:00:00"), 3600, "eth2", 1, 10, "");

 // Entries for earlier this month
    addDbRow(makeTs("2009-03-01 10:00:00"), 3600, "eth0", 1, 10, "");
    addDbRow(makeTs("2009-03-01 11:00:00"), 3600, "eth1", 1, 10, "host2");

 // Entries for earlier this year
    addDbRow(makeTs("2009-02-01 10:00:00"), 3600, "eth0", 1, 10, "host1");
    addDbRow(makeTs("2009-01-01 11:00:00"), 3600, "eth1", 1, 10, "host1");

 // Entries for previous years
    addDbRow(makeTs("2008-12-31 10:00:00"), 3600, "eth0", 1, 10, "");
    addDbRow(makeTs("2007-01-01 11:00:00"), 3600, "eth1", 1, 10, "host2");

    char* hosts[2] = {"host1", "host2"};
    checkSummary(tc, getSummaryValues(NULL, NULL),      makeTs("2007-01-01 11:00:00"), makeTs("2009-03-02 10:00:00"), 6, 60, 8, 80, 10, 100, 12, 120, 2, hosts);
    checkSummary(tc, getSummaryValues("", NULL),        makeTs("2008-12-31 10:00:00"), makeTs("2009-03-02 10:00:00"), 4, 40, 5, 50, 5, 50, 6, 60, 0, NULL);
    checkSummary(tc, getSummaryValues("host1", NULL),   makeTs("2009-01-01 11:00:00"), makeTs("2009-03-02 09:59:59"), 2, 20, 2, 20, 4, 40, 4, 40, 0, NULL);
    checkSummary(tc, getSummaryValues("host2", NULL),   makeTs("2007-01-01 11:00:00"), makeTs("2009-03-01 11:00:00"), 0, 0, 1, 10, 1, 10, 2, 20, 0, NULL);
    checkSummary(tc, getSummaryValues("", "eth0"),      makeTs("2008-12-31 10:00:00"), makeTs("2009-03-02 08:00:00"), 1, 10, 2, 20, 2, 20, 3, 30, 0, NULL);
    checkSummary(tc, getSummaryValues("", "eth1"),      makeTs("2009-03-02 07:59:00"), makeTs("2009-03-02 10:00:00"), 2, 20, 2, 20, 2, 20, 2, 20, 0, NULL);
    checkSummary(tc, getSummaryValues("", "eth2"),      makeTs("2009-03-02 01:00:00"), makeTs("2009-03-02 01:00:00"), 1, 10, 1, 10, 1, 10, 1, 10, 0, NULL);
    checkSummary(tc, getSummaryValues("host1", "eth0"), makeTs("2009-02-01 10:00:00"), makeTs("2009-03-02 09:59:59"), 2, 20, 2, 20, 3, 30, 3, 30, 0, NULL);
}

void testSummaryOneOtherHost(CuTest *tc) {
    time_t now = makeTs("2009-01-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 1, 1, "server");
    addDbRow(now, 1, "eth1", 1, 1, "");
    addDbRow(now, 1, "eth0", 1, 1, "server");
    char* hosts[1] = {"server"};
    checkSummary(tc, getSummaryValues(NULL, NULL), now, now, 3, 3, 3, 3, 3, 3, 3, 3, 1, hosts);
}

void testSummaryMultipleOtherHosts(CuTest *tc) {
    time_t now = makeTs("2009-01-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0",   1, 1, "server1");
    addDbRow(now, 1, "eth1",   1, 1, "");
    addDbRow(now, 1, "eth0",   1, 1, "server2");
    addDbRow(now, 1, "random", 1, 1, "server3");
    addDbRow(now, 1, "eth0",   1, 1, "server1");
    addDbRow(now, 1, "eth1",   1, 1, "server1");

    char* hosts[3] = {"server1", "server2", "server3"};

    checkSummary(tc, getSummaryValues(NULL, NULL),        now, now, 6, 6, 6, 6, 6, 6, 6, 6, 3, hosts);
    checkSummary(tc, getSummaryValues("server1", NULL),   now, now, 3, 3, 3, 3, 3, 3, 3, 3, 0, NULL);
    checkSummary(tc, getSummaryValues("server2", NULL),   now, now, 1, 1, 1, 1, 1, 1, 1, 1, 0, NULL);
    checkSummary(tc, getSummaryValues("server3", NULL),   now, now, 1, 1, 1, 1, 1, 1, 1, 1, 0, NULL);
    checkSummary(tc, getSummaryValues("", NULL),          now, now, 1, 1, 1, 1, 1, 1, 1, 1, 0, NULL);
    checkSummary(tc, getSummaryValues("badserver", NULL), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
    checkSummary(tc, getSummaryValues("server1", "eth0"), now, now, 2, 2, 2, 2, 2, 2, 2, 2, 0, NULL);
    checkSummary(tc, getSummaryValues("server1", "eth1"), now, now, 1, 1, 1, 1, 1, 1, 1, 1, 0, NULL);
    checkSummary(tc, getSummaryValues("server1", "eth2"), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
}

static void checkSummary(CuTest *tc, struct Summary summary, time_t tsMin, time_t tsMax,
        BW_INT todayDl, BW_INT todayUl, BW_INT monthDl, BW_INT monthUl, BW_INT yearDl, BW_INT yearUl, BW_INT totalDl, BW_INT totalUl,
        int hostCount, char** hostNames){
 // Helper function used to verify the contents of a Summary struct
    CuAssertIntEquals(tc, tsMin,     summary.tsMin);
    CuAssertIntEquals(tc, tsMax,     summary.tsMax);
    CuAssertIntEquals(tc, todayDl,   summary.today->dl);
    CuAssertIntEquals(tc, todayUl,   summary.today->ul);
    CuAssertIntEquals(tc, monthDl,   summary.month->dl);
    CuAssertIntEquals(tc, monthUl,   summary.month->ul);
    CuAssertIntEquals(tc, yearDl,    summary.year->dl);
    CuAssertIntEquals(tc, yearUl,    summary.year->ul);
    CuAssertIntEquals(tc, totalDl,   summary.total->dl);
    CuAssertIntEquals(tc, totalUl,   summary.total->ul);
    CuAssertIntEquals(tc, hostCount, summary.hostCount);

    int i;
    for(i=0; i<hostCount; i++){
        CuAssertStrEquals(tc, hostNames[i], summary.hostNames[i]);
    }
}

CuSuite* clientSummaryGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testSummaryEmptyDb);
    SUITE_ADD_TEST(suite, testSummaryOneEntry);
    SUITE_ADD_TEST(suite, testSummaryTwoEntriesSameTime);
    SUITE_ADD_TEST(suite, testSummaryTwoEntriesDifferentTimes);
    SUITE_ADD_TEST(suite, testSummaryEntriesSpanningDayBoundary);
    SUITE_ADD_TEST(suite, testSummaryEntriesSpanningMonthBoundary);
    SUITE_ADD_TEST(suite, testSummaryEntriesSpanningYearBoundary);
    SUITE_ADD_TEST(suite, testSummaryMultipleEntries);
    SUITE_ADD_TEST(suite, testSummaryOneOtherHost);
    SUITE_ADD_TEST(suite, testSummaryMultipleOtherHosts);
    return suite;
}
