#include "test.h"
#include "common.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "capture.h"
#include "CuTest.h"

static int makeTs(const char* dateTxt);

void testGetCurrentYearForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-01-01 00:00:00"), getCurrentYearForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentYearForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentYearForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentYearForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetCurrentMonthForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-05-01 00:00:00"), getCurrentMonthForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentMonthForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-01 00:00:00"), getCurrentMonthForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-12-01 00:00:00"), getCurrentMonthForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetCurrentDayForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-05-26 00:00:00"), getCurrentDayForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:00:00"), getCurrentDayForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-24 00:00:00"), getCurrentDayForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2009-12-31 00:00:00"), getCurrentDayForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextYearForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1971-01-01 00:00:00"), getNextYearForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextYearForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextYearForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextYearForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextMonthForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-06-01 00:00:00"), getNextMonthForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-02-01 00:00:00"), getNextMonthForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-04-01 00:00:00"), getNextMonthForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextMonthForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextDayForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-05-27 00:00:00"), getNextDayForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-02 00:00:00"), getNextDayForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-25 00:00:00"), getNextDayForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextDayForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextHourForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-05-26 11:00:00"), getNextHourForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 01:00:00"), getNextHourForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-24 20:00:00"), getNextHourForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextHourForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetNextMinForTs(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-05-26 10:02:00"), getNextMinForTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, makeTs("2009-01-01 00:01:00"), getNextMinForTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, makeTs("2009-03-24 19:13:00"), getNextMinForTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, makeTs("2010-01-01 00:00:00"), getNextMinForTs(makeTs("2009-12-31 23:59:59")));
}

void testGetYearFromTs(CuTest *tc){
    CuAssertIntEquals(tc, 1970, getYearFromTs(makeTs("1970-05-26 10:01:00")));
    CuAssertIntEquals(tc, 2009, getYearFromTs(makeTs("2009-01-01 00:00:00")));
    CuAssertIntEquals(tc, 2009, getYearFromTs(makeTs("2009-03-24 19:12:01")));
    CuAssertIntEquals(tc, 2009, getYearFromTs(makeTs("2009-12-31 23:59:59")));
}

void testAddToDate(CuTest *tc){
    CuAssertIntEquals(tc, makeTs("1970-05-26 11:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'h', 1));
    CuAssertIntEquals(tc, makeTs("1970-05-28 10:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'd', 2));
    CuAssertIntEquals(tc, makeTs("1970-08-26 10:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'm', 3));
    CuAssertIntEquals(tc, makeTs("1974-05-26 10:01:00"), addToDate(makeTs("1970-05-26 10:01:00"), 'y', 4));
}

static int makeTs(const char* dateTxt){
    struct tm t;
    strptime(dateTxt, "%Y-%m-%d %H:%M:%S", &t);
    return mktime(&t);
}

CuSuite* timeGetSuite() {
    putenv("TZ=GMT"); // Need this because the date/time fns use localtime
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testGetCurrentYearForTs);
    SUITE_ADD_TEST(suite, testGetCurrentMonthForTs);
    SUITE_ADD_TEST(suite, testGetCurrentDayForTs);
    SUITE_ADD_TEST(suite, testGetNextYearForTs);
    SUITE_ADD_TEST(suite, testGetNextMonthForTs);
    SUITE_ADD_TEST(suite, testGetNextDayForTs);
    SUITE_ADD_TEST(suite, testGetNextHourForTs);
    SUITE_ADD_TEST(suite, testGetNextMinForTs);
    SUITE_ADD_TEST(suite, testGetYearFromTs);
    SUITE_ADD_TEST(suite, testAddToDate);
    return suite;
}
