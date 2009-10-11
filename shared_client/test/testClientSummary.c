#include "test.h"
#include "common.h"
#include "client.h"
#include "CuTest.h"

static void checkSummary(CuTest *, struct Summary, time_t, time_t, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT, BW_INT);

void testSummaryEmptyDb(CuTest *tc) {
    emptyDb();
    checkSummary(tc, getSummaryValues(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void testSummaryOneEntry(CuTest *tc) {
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 123, 456);
    checkSummary(tc, getSummaryValues(), now, now, 123, 456, 123, 456, 123, 456, 123, 456);
}

void testSummaryTwoEntriesSameTime(CuTest *tc) {
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 100, 400);
    addDbRow(now, 1, "eth1", 23, 56);
    checkSummary(tc, getSummaryValues(), now, now, 123, 456, 123, 456, 123, 456, 123, 456);
}

void testSummaryTwoEntriesDifferentTimes(CuTest *tc) {
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 100, 400);
    addDbRow(now - 2, 1, "eth1", 23, 56);
    checkSummary(tc, getSummaryValues(), now - 2, now - 1, 123, 456, 123, 456, 123, 456, 123, 456);
}

void testSummaryEntriesSpanningDayBoundary(CuTest *tc) {
    time_t now = makeTs("2009-01-02 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10); //today
    addDbRow(now - 1, 1, "eth1", 2, 11); //today
    addDbRow(now - 2, 1, "eth0", 4, 12); //yesterday
    checkSummary(tc, getSummaryValues(), now - 2, now, 3, 21, 7, 33, 7, 33, 7, 33);
}

void testSummaryEntriesSpanningMonthBoundary(CuTest *tc) {
    time_t now = makeTs("2009-02-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10); //today
    addDbRow(now - 1, 1, "eth1", 2, 11); //today
    addDbRow(now - 2, 1, "eth0", 4, 12); //yesterday and last month
    checkSummary(tc, getSummaryValues(), now - 2, now, 3, 21, 3, 21, 7, 33, 7, 33);
}

void testSummaryEntriesSpanningYearBoundary(CuTest *tc) {
    time_t now = makeTs("2009-01-01 00:00:01");
    setTime(now);
    emptyDb();
    addDbRow(now,     1, "eth0", 1, 10); //today
    addDbRow(now - 1, 1, "eth1", 2, 11); //today
    addDbRow(now - 2, 1, "eth0", 4, 12); //yesterday and last year
    checkSummary(tc, getSummaryValues(), now - 2, now, 3, 21, 3, 21, 3, 21, 7, 33);
}

void testSummaryMultipleEntries(CuTest *tc) {
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
