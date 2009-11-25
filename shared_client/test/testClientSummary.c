/*
 * BitMeterOS v0.2.0
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
 * Build Date: Wed, 25 Nov 2009 10:48:23 +0000
 */

#include "test.h"
#include "common.h"
#include "client.h"
#include "CuTest.h"

/*
Contains unit tests for the clientSummary module.
*/

static void checkSummary(CuTest *, struct Summary, time_t, time_t, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT);

void testSummaryEmptyDb(CuTest *tc) {
 // Check that we behave correctly when the data table is empty	
    emptyDb();
    checkSummary(tc, getSummaryValues(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void testSummaryOneEntry(CuTest *tc) {
 // Check that we behave correctly when the data table contains only 1 row
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 123, 456);
    checkSummary(tc, getSummaryValues(), now, now, 123, 456, 123, 456, 123, 456, 123, 456);
}

void testSummaryTwoEntriesSameTime(CuTest *tc) {
 // Check that we behave correctly when the data table contains 2 entries with the same timestamp
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 100, 400);
    addDbRow(now, 1, "eth1", 23, 56);
    checkSummary(tc, getSummaryValues(), now, now, 123, 456, 123, 456, 123, 456, 123, 456);
}

void testSummaryTwoEntriesDifferentTimes(CuTest *tc) {
 // Check that we behave correctly when the data table contains 2 entries with different timestamps
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 100, 400);
    addDbRow(now - 2, 1, "eth1", 23, 56);
    checkSummary(tc, getSummaryValues(), now - 2, now - 1, 123, 456, 123, 456, 123, 456, 123, 456);
}

void testSummaryEntriesSpanningDayBoundary(CuTest *tc) {
 // Check that results are correct when our data spans a day boundary
    time_t now = makeTs("2009-01-02 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10); //today
    addDbRow(now - 1, 1, "eth1", 2, 11); //today
    addDbRow(now - 2, 1, "eth0", 4, 12); //yesterday
    checkSummary(tc, getSummaryValues(), now - 2, now, 3, 21, 7, 33, 7, 33, 7, 33);
}

void testSummaryEntriesSpanningMonthBoundary(CuTest *tc) {
 // Check that results are correct when our data spans a month boundary	
    time_t now = makeTs("2009-02-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10); //today
    addDbRow(now - 1, 1, "eth1", 2, 11); //today
    addDbRow(now - 2, 1, "eth0", 4, 12); //yesterday and last month
    checkSummary(tc, getSummaryValues(), now - 2, now, 3, 21, 3, 21, 7, 33, 7, 33);
}

void testSummaryEntriesSpanningYearBoundary(CuTest *tc) {
 // Check that results are correct when our data spans a year boundary	
    time_t now = makeTs("2009-01-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10); //today
    addDbRow(now - 1, 1, "eth1", 2, 11); //today
    addDbRow(now - 2, 1, "eth0", 4, 12); //yesterday and last year
    checkSummary(tc, getSummaryValues(), now - 2, now, 3, 21, 3, 21, 3, 21, 7, 33);
}

void testSummaryMultipleEntries(CuTest *tc) {
 // Check that results are correct when we have multiple entries spread over a large date range
    time_t now = makeTs("2009-03-02 10:00:00");
    setTime(now);
    emptyDb();

 // Entries for today
    addDbRow(makeTs("2009-03-02 10:00:00"), 1,    "eth1", 1, 10);
    addDbRow(makeTs("2009-03-02 09:59:59"), 1,    "eth0", 1, 10);
    addDbRow(makeTs("2009-03-02 08:00:00"), 60,   "eth0", 1, 10);
    addDbRow(makeTs("2009-03-02 07:59:00"), 60,   "eth1", 1, 10);
    addDbRow(makeTs("2009-03-02 07:58:00"), 60,   "eth0", 1, 10);
    addDbRow(makeTs("2009-03-02 01:00:00"), 3600, "eth2", 1, 10);

 // Entries for earlier this month
    addDbRow(makeTs("2009-03-01 10:00:00"), 3600, "eth0", 1, 10);
    addDbRow(makeTs("2009-03-01 11:00:00"), 3600, "eth1", 1, 10);

 // Entries for earlier this year
    addDbRow(makeTs("2009-02-01 10:00:00"), 3600, "eth0", 1, 10);
    addDbRow(makeTs("2009-01-01 11:00:00"), 3600, "eth1", 1, 10);

 // Entries for previous years
    addDbRow(makeTs("2008-12-31 10:00:00"), 3600, "eth0", 1, 10);
    addDbRow(makeTs("2007-01-01 11:00:00"), 3600, "eth1", 1, 10);

    checkSummary(tc, getSummaryValues(), makeTs("2007-01-01 11:00:00"), now, 6, 60, 8, 80, 10, 100, 12, 120);
}

static void checkSummary(CuTest *tc, struct Summary summary, time_t tsMin, time_t tsMax,
        BW_INT todayDl, BW_INT todayUl, BW_INT monthDl, BW_INT monthUl, BW_INT yearDl, BW_INT yearUl, BW_INT totalDl, BW_INT totalUl){
 // Helper function used to verify the contents of a Summary struct        	
    CuAssertIntEquals(tc, tsMin,   summary.tsMin);
    CuAssertIntEquals(tc, tsMax,   summary.tsMax);
    CuAssertIntEquals(tc, todayDl, summary.today->dl);
    CuAssertIntEquals(tc, todayUl, summary.today->ul);
    CuAssertIntEquals(tc, monthDl, summary.month->dl);
    CuAssertIntEquals(tc, monthUl, summary.month->ul);
    CuAssertIntEquals(tc, yearDl,  summary.year->dl);
    CuAssertIntEquals(tc, yearUl,  summary.year->ul);
    CuAssertIntEquals(tc, totalDl, summary.total->dl);
    CuAssertIntEquals(tc, totalUl, summary.total->ul);
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
    return suite;
}
