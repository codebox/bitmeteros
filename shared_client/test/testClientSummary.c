#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "client.h"

/*
Contains unit tests for the clientSummary module.
*/

static void checkSummaryMain(struct Summary summary, time_t tsMin, time_t tsMax, int hostCount, char** hostNames);
static void checkSummaryFilter(struct Summary summary, int fl, BW_INT todayVal, BW_INT monthVal, BW_INT yearVal, BW_INT totalVal);

void testSummaryEmptyDb(void** state) {
 // Check that we behave correctly when the data table is empty
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, 0, 0, 0, NULL);
    freeSummary(&summary);
    freeStmtList();
}

void testSummaryOneEntry(void** state) {
 // Check that we behave correctly when the data table contains only 1 row
    time_t now = makeTsUtc("2009-01-01 10:00:00");
    setTime(now);
    addDbRow(now, 1, 100, FILTER);
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now, now, 0, NULL);
    checkSummaryFilter(summary, FILTER, 100, 100, 100, 100);
    freeSummary(&summary);    
    freeStmtList();
}

void testSummaryTwoEntriesSameTime(void** state) {
 // Check that we behave correctly when the data table contains 2 entries with the same timestamp
    time_t now = makeTsUtc("2009-01-01 10:00:00");
    setTime(now);
    addDbRow(now, 1, 100, FILTER);
    addDbRow(now, 1, 1, FILTER);
    
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now, now, 0, NULL);
    checkSummaryFilter(summary, FILTER, 101, 101, 101, 101);
    freeSummary(&summary);    
    freeStmtList();
}

void testSummaryTwoEntriesDifferentTimes(void** state) {
 // Check that we behave correctly when the data table contains 2 entries with different timestamps
    time_t now = makeTsUtc("2009-01-01 10:00:00");
    setTime(now);
    addDbRow(now-1, 1, 100, FILTER);
    addDbRow(now-2, 1, 1, FILTER);
    
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now-2, now-1, 0, NULL);
    checkSummaryFilter(summary, FILTER, 101, 101, 101, 101);
    freeSummary(&summary);  
    freeStmtList();  
}

void testSummaryEntriesSpanningDayBoundary(void** state) {
 // Check that results are correct when our data spans a day boundary
    time_t now = makeTsUtc("2009-01-02 00:00:01");
    setTime(now);
    addDbRow(now,     1, 1, FILTER); //today
    addDbRow(now - 1, 1, 2, FILTER); //today
    addDbRow(now - 2, 1, 4, FILTER); //yesterday
    
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now-2, now, 0, NULL);
    checkSummaryFilter(summary, FILTER, 3, 7, 7, 7);
    freeSummary(&summary);    
    freeStmtList();
}

void testSummaryEntriesSpanningMonthBoundary(void** state) {
 // Check that results are correct when our data spans a month boundary
    time_t now = makeTsUtc("2009-02-01 00:00:01");
    setTime(now);
    
    addDbRow(now,     1, 1, FILTER); //today
    addDbRow(now - 1, 1, 2, FILTER); //today
    addDbRow(now - 2, 1, 4, FILTER); //yesterday and last month

    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now-2, now, 0, NULL);
    checkSummaryFilter(summary, FILTER, 3, 3, 7, 7);
    freeSummary(&summary); 
    freeStmtList();   
}

void testSummaryEntriesSpanningYearBoundary(void** state) {
 // Check that results are correct when our data spans a year boundary
    time_t now = makeTsUtc("2009-01-01 00:00:01");
    setTime(now);

    addDbRow(now,     1, 1, FILTER); //today
    addDbRow(now - 1, 1, 2, FILTER); //today
    addDbRow(now - 2, 1, 4, FILTER); //yesterday and last year
    
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now-2, now, 0, NULL);
    checkSummaryFilter(summary, FILTER, 3, 3, 3, 7);
    freeSummary(&summary);    
    freeStmtList();
}

void testSummaryMultipleEntries(void** state) {
 // Check that results are correct when we have multiple entries spread over a large date range
    time_t now = makeTsUtc("2009-03-02 10:00:00");
    setTime(now);
    
 // Set up filters for other hosts
    addFilterRow(FILTER,  "1", "1", "", NULL);
    addFilterRow(FILTER2, "2", "2", "", "host1");
    addFilterRow(FILTER3, "3", "3", "", "host2");
    
 // Entries for today
    addDbRow(makeTsUtc("2009-03-02 10:00:00"), 1,    1, FILTER);
    addDbRow(makeTsUtc("2009-03-02 09:59:59"), 1,    1, FILTER2);
    addDbRow(makeTsUtc("2009-03-02 08:00:00"), 60,   1, FILTER);
    addDbRow(makeTsUtc("2009-03-02 07:59:00"), 60,   1, FILTER);
    addDbRow(makeTsUtc("2009-03-02 07:58:00"), 60,   1, FILTER2);
    addDbRow(makeTsUtc("2009-03-02 01:00:00"), 3600, 1, FILTER);
    
 // Entries for earlier this month
    addDbRow(makeTsUtc("2009-03-01 10:00:00"), 3600, 1, FILTER);
    addDbRow(makeTsUtc("2009-03-01 11:00:00"), 3600, 1, FILTER3);
    
 // Entries for earlier this year
    addDbRow(makeTsUtc("2009-02-01 10:00:00"), 3600, 1, FILTER2);
    addDbRow(makeTsUtc("2009-01-01 11:00:00"), 3600, 1, FILTER2);
    
 // Entries for previous years
    addDbRow(makeTsUtc("2008-12-31 10:00:00"), 3600, 1, FILTER);
    addDbRow(makeTsUtc("2007-01-01 11:00:00"), 3600, 1, FILTER3);
    
    struct Summary summary;
    char* hosts[2] = {"host1", "host2"};
    summary = getSummaryValues();
    checkSummaryMain(summary, makeTsUtc("2007-01-01 11:00:00"), makeTsUtc("2009-03-02 10:00:00"), 2, &hosts);
    checkSummaryFilter(summary, FILTER,  4, 5, 5, 6);
    checkSummaryFilter(summary, FILTER2, 2, 2, 4, 4);
    checkSummaryFilter(summary, FILTER3, 0, 1, 1, 2);
    freeSummary(&summary);    
    freeStmtList();
}

void testSummaryOneOtherHost(void** state) {
    time_t now = makeTsUtc("2009-01-01 00:00:01");
    setTime(now);
    
 // Set up filters for other hosts
    addFilterRow(FILTER,  "1", "1", "", NULL);
    addFilterRow(FILTER2, "2", "2", "", "server");

    addDbRow(now, 1, 1, FILTER2);
    addDbRow(now, 1, 1, FILTER);
    addDbRow(now, 1, 1, FILTER2);
    char* hosts[1] = {"server"};
    
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now, now, 1, hosts);
    checkSummaryFilter(summary, FILTER,  1, 1, 1, 1);
    checkSummaryFilter(summary, FILTER2, 2, 2, 2, 2);
    freeSummary(&summary);  
    freeStmtList();  
}

void testSummaryMultipleOtherHosts(void** state) {
    time_t now = makeTsUtc("2009-01-01 00:00:01");
    setTime(now);

 // Set up filters for other hosts
    addFilterRow(FILTER,  "1", "1", "", NULL);
    addFilterRow(FILTER2, "2", "2", "", "server1");
    addFilterRow(FILTER3, "3", "3", "", "server2");
    addFilterRow(FILTER4, "4", "4", "", "server3");

    addDbRow(now, 1, 1, FILTER2);
    addDbRow(now, 1, 1, FILTER); 
    addDbRow(now, 1, 1, FILTER3);
    addDbRow(now, 1, 1, FILTER4);
    addDbRow(now, 1, 1, FILTER2);
    addDbRow(now, 1, 1, FILTER2);
    
    char* hosts[3] = {"server1", "server2", "server3"};
    
    struct Summary summary = getSummaryValues();
    checkSummaryMain(summary, now, now, 3, hosts);
    checkSummaryFilter(summary, FILTER,  1, 1, 1, 1);
    checkSummaryFilter(summary, FILTER2, 3, 3, 3, 3);
    checkSummaryFilter(summary, FILTER3, 1, 1, 1, 1);
    checkSummaryFilter(summary, FILTER4, 1, 1, 1, 1);
    freeSummary(&summary);
    freeStmtList();    
}

static void checkSummaryMain(struct Summary summary, time_t tsMin, time_t tsMax, int hostCount, char** hostNames){
 // Helper function used to verify the contents of a Summary struct
    assert_int_equal(tsMin,     summary.tsMin);
    assert_int_equal(tsMax,     summary.tsMax);
    assert_int_equal(hostCount, summary.hostCount);

    int i;
    for(i=0; i<hostCount; i++){
        assert_string_equal(hostNames[i], summary.hostNames[i]);
    }
}

static void checkSummaryFilter(struct Summary summary, int fl, BW_INT todayVal, 
        BW_INT monthVal, BW_INT yearVal, BW_INT totalVal){
 // Helper function used to verify the contents of a Summary struct
    assert_int_equal(todayVal, getValueForFilterId(summary.today, fl));
    assert_int_equal(monthVal, getValueForFilterId(summary.month, fl));
    assert_int_equal(yearVal,  getValueForFilterId(summary.year, fl));
    assert_int_equal(totalVal, getValueForFilterId(summary.total, fl));
}
