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
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "CuTest.h"

/*
Contains unit tests for the 'time' module.
*/

#ifdef _WIN32
void makeTm(struct tm *t, int year, int month, int day, int hour, int min, int sec){
	t->tm_sec  = sec;
	t->tm_min  = min;
	t->tm_hour = hour;
	t->tm_mday = day;
	t->tm_mon  = month - 1;
	t->tm_year = year - 1900;
	t->tm_isdst = -1;
}
void testTimeGm(CuTest *tc){
	struct tm t;
	
	makeTm(&t, 2010, 1, 1, 0, 0, 0);
	CuAssertIntEquals(tc, 1262304000, timegm(&t));

	makeTm(&t, 2010, 7, 1, 0, 0, 0);
	CuAssertIntEquals(tc, 1277942400, timegm(&t));
}
#endif

void testGetCurrentYearForTs(CuTest *tc){
 // Check that the 'getCurrentYearForTs' function correctly calculates the start of the current year for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-01-01 00:00:00"), getCurrentLocalYearForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentLocalYearForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentLocalYearForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentLocalYearForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetCurrentMonthForTs(CuTest *tc){
 // Check that the 'getCurrentMonthForTs' function correctly calculates the start of the current month for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-05-01 00:00:00"), getCurrentLocalMonthForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentLocalMonthForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-01 00:00:00"), getCurrentLocalMonthForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-12-01 00:00:00"), getCurrentLocalMonthForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetCurrentDayForTs(CuTest *tc){
 // Check that the 'getCurrentDayForTs' function correctly calculates the start of the current day for various timestamps
    CuAssertIntEquals(tc, makeTs("1970-05-26 00:00:00"), getCurrentLocalDayForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentLocalDayForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-24 00:00:00"), getCurrentLocalDayForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-12-31 00:00:00"), getCurrentLocalDayForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextYearForTs(CuTest *tc){
 // Check that the 'getNextYearForTs' function correctly calculates the start of the next year for various timestamps
    CuAssertIntEquals(tc, makeTsUtc("1971-01-01 00:00:00"), getNextYearForTs(makeTsUtc("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTsUtc("2010-01-01 00:00:00"), getNextYearForTs(makeTsUtc("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTsUtc("2010-01-01 00:00:00"), getNextYearForTs(makeTsUtc("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTsUtc("2010-01-01 00:00:00"), getNextYearForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextMonthForTs(CuTest *tc){
 // Check that the 'getNextMonthForTs' function correctly calculates the start of the next month for various timestamps
    CuAssertIntEquals(tc, makeTsUtc("1970-06-01 00:00:00"), getNextMonthForTs(makeTsUtc("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-02-01 00:00:00"), getNextMonthForTs(makeTsUtc("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-04-01 00:00:00"), getNextMonthForTs(makeTsUtc("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTsUtc("2010-01-01 00:00:00"), getNextMonthForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextDayForTs(CuTest *tc){
 // Check that the 'getNextDayForTs' function correctly calculates the start of the next day for various timestamps
    CuAssertIntEquals(tc, makeTsUtc("1970-05-27 00:00:00"), getNextDayForTs(makeTsUtc("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-01-02 00:00:00"), getNextDayForTs(makeTsUtc("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-03-25 00:00:00"), getNextDayForTs(makeTsUtc("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTsUtc("2010-01-01 00:00:00"), getNextDayForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextHourForTs(CuTest *tc){
 // Check that the 'getNextHourForTs' function correctly calculates the start of the next hour for various timestamps
    CuAssertIntEquals(tc, makeTsUtc("1970-05-26 11:00:00"), getNextHourForTs(makeTsUtc("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-01-01 01:00:00"), getNextHourForTs(makeTsUtc("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-03-24 20:00:00"), getNextHourForTs(makeTsUtc("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTsUtc("2010-01-01 00:00:00"), getNextHourForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextMinForTs(CuTest *tc){
 // Check that the 'getNextMinForTs' function correctly calculates the start of the next minute for various timestamps
    CuAssertIntEquals(tc, makeTsUtc("1970-05-26 10:02:00"), getNextMinForTs(makeTsUtc("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-01-01 00:01:00"), getNextMinForTs(makeTsUtc("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTsUtc("2009-03-24 19:13:00"), getNextMinForTs(makeTsUtc("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTsUtc("2010-01-01 00:00:00"), getNextMinForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testAddToDate(CuTest *tc){
 // Check that the 'addToDate' function correctly adds various different values to a timestamp
    CuAssertIntEquals(tc, makeTsUtc("1970-05-26 11:01:00"), addToDate(makeTsUtc("1970-05-26 10:01:00"), 'h', 1));
    CuAssertIntEquals(tc, makeTsUtc("1970-05-28 10:01:00"), addToDate(makeTsUtc("1970-05-26 10:01:00"), 'd', 2));
    CuAssertIntEquals(tc, makeTsUtc("1970-08-26 10:01:00"), addToDate(makeTsUtc("1970-05-26 10:01:00"), 'm', 3));
    CuAssertIntEquals(tc, makeTsUtc("1974-05-26 10:01:00"), addToDate(makeTsUtc("1970-05-26 10:01:00"), 'y', 4));
}

CuSuite* timeGetSuite() {
    CuSuite* suite = CuSuiteNew();
    #ifdef _WIN32
		SUITE_ADD_TEST(suite, testTimeGm);
	#endif
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
