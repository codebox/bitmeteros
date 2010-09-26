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
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "CuTest.h"

/*
Contains unit tests for the 'time' module.
*/

void testGetCurrentYearForTs(CuTest *tc){
 // Check that the 'getCurrentYearForTs' function correctly calculates the start of the current year for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-01-01 00:00:00"), getCurrentYearForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentYearForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentYearForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentYearForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetCurrentMonthForTs(CuTest *tc){
 // Check that the 'getCurrentMonthForTs' function correctly calculates the start of the current month for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-05-01 00:00:00"), getCurrentMonthForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentMonthForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-01 00:00:00"), getCurrentMonthForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-12-01 00:00:00"), getCurrentMonthForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetCurrentDayForTs(CuTest *tc){
 // Check that the 'getCurrentDayForTs' function correctly calculates the start of the current day for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-05-26 00:00:00"), getCurrentDayForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentDayForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-24 00:00:00"), getCurrentDayForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-12-31 00:00:00"), getCurrentDayForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextYearForTs(CuTest *tc){
 // Check that the 'getNextYearForTs' function correctly calculates the start of the next year for various timestamps
    CuAssertIntEquals(tc, makeTs("1971-01-01 00:00:00"), getNextYearForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextYearForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextYearForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextYearForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextMonthForTs(CuTest *tc){
 // Check that the 'getNextMonthForTs' function correctly calculates the start of the next month for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-06-01 00:00:00"), getNextMonthForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-02-01 00:00:00"), getNextMonthForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-04-01 00:00:00"), getNextMonthForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextMonthForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextDayForTs(CuTest *tc){
 // Check that the 'getNextDayForTs' function correctly calculates the start of the next day for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-05-27 00:00:00"), getNextDayForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-02 00:00:00"), getNextDayForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-25 00:00:00"), getNextDayForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextDayForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextHourForTs(CuTest *tc){
 // Check that the 'getNextHourForTs' function correctly calculates the start of the next hour for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-05-26 11:00:00"), getNextHourForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 01:00:00"), getNextHourForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-24 20:00:00"), getNextHourForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextHourForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextMinForTs(CuTest *tc){
 // Check that the 'getNextMinForTs' function correctly calculates the start of the next minute for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-05-26 10:02:00"), getNextMinForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:01:00"), getNextMinForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-24 19:13:00"), getNextMinForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextMinForTs(makeTs("2009-12-31 23:59:59")));
}

void testAddToDate(CuTest *tc){
 // Check that the 'addToDate' function correctly adds various different values to a timestamp
    CuAssertIntEquals(tc, makeTs("1970-05-26 11:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'h', 1));
    CuAssertIntEquals(tc, makeTs("1970-05-28 10:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'd', 2));
    CuAssertIntEquals(tc, makeTs("1970-08-26 10:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'm', 3));
    CuAssertIntEquals(tc, makeTs("1974-05-26 10:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'y', 4));
}

CuSuite* timeGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testGetCurrentYearForTs);
    SUITE_ADD_TEST(suite, testGetCurrentMonthForTs);
    SUITE_ADD_TEST(suite, testGetCurrentDayForTs);
    SUITE_ADD_TEST(suite, testGetNextYearForTs);
    SUITE_ADD_TEST(suite, testGetNextMonthForTs);
    SUITE_ADD_TEST(suite, testGetNextDayForTs);
    SUITE_ADD_TEST(suite, testGetNextHourForTs);
    SUITE_ADD_TEST(suite, testGetNextMinForTs);
    SUITE_ADD_TEST(suite, testAddToDate);
    return suite;
}
