#include "common.h"
#include <time.h>
#include <stdio.h>

#define _BSD_SOURCE 1

#define malloc(size)          _test_malloc(size, __FILE__, __LINE__) 
#define calloc(num, size)     _test_calloc(num, size, __FILE__, __LINE__) 
#define free(ptr)             _test_free(ptr, __FILE__, __LINE__) 
#define strdup(ptr)           _test_strdup(ptr, __FILE__, __LINE__) 
#define printf(fmt , args...) _test_printf(fmt , ##args) 
#define dbg(fmt , args...) fprintf(stdout , fmt , ##args) 

#define expect_call(function) expect_any(function, dummy)

#define FILTER  1
#define FILTER2 2
#define FILTER3 3
#define FILTER4 4

void setupTestDb(void** state);
void tearDownTestDb(void** state);
void addDbRow(time_t ts, int dr, int vl, int fl);
void addFilterRow(int id, char* desc, char* name, char* expr, char* host);
void rmConfigRow(char* key);
void addConfigRow(char* key, char* value);
void checkFilter(struct Filter* filter, int id, char* desc, char* name, char* expr, char* host);
void setupTestForHandleConfig(void** state);
void teardownTestForHandleConfig(void** state);
void setupTestForHandleSummary(void** state);
void tearDownTestForHandleSummary(void** state);
void setupTestForHandleFile(void** state);
void tearDownTestForHandleFile(void** state);
void setupTestForHandleQuery(void** state);
void tearDownTestForHandleQuery(void** state);
void setupTestForHandleMonitor(void** state);
void tearDownTestForHandleMonitor(void** state);
void setupTestForHandleAlert(void** state);
void tearDownTestForHandleAlert(void** state);
void setupForProcessTest(void** state);
void teardownForProcessTest(void** state);
void setupTestForTotal(void** state);
void teardownTestForTotal(void** state);
void dumpDataTable();
int tableHasColumn(char* table, char* column);

// filter.c
void testCopyFilter(void** state);
void testFilterHasHost(void** state);
void testAllocFilter(void **state);
void testAllocFilterWithNulls(void **state);
void testFreeFilter(void **state);
void testGetFilterFromId(void **state);
void testGetFilterFromName(void **state);
void testAppendFilter(void **state);
void testGetTotalForFilter(void **state);
void testGetMaxFilterDescWidth(void **state);
void testGetMaxFilterNameWidth(void **state);

// adapter.c
void testAllocAdapter(void **state);
void testFreeAdapters(void **state);
void testAppendAdapter(void **state);

// alert.c
void testAllocAlert(void **state);
void testSetAlertName(void **state);
void testAppendAlert(void **state);
void testMakeDateCriteriaPart(void **state);
void testDateCriteriaPartToText(void **state);
void testMakeDateCriteria(void **state);
void testAppendDateCriteria(void **state);

// common.c
void testFormatAmounts(void **state);
void testToTime(void **state);
void testToDate(void **state);
void testMakeHexString(void **state);
void testStrToLong(void **state);
void testReplace(void** state);

// clientDump.c
void testClientDumpEmptyDb(void **state);
void testClientDumpOneEntry(void **state);
void testClientDumpMultipleEntries(void **state);

// bmclient.c
void testMainHelp(void** state);
void testMainVersion(void** state);
void testMainFailWithError(void** state);
void testMainFailNoError(void** state);
void testMainDoDump(void** state);
void testMainDoSummary(void** state);
void testMainDoQuery(void** state);
void testMainDoMonitor(void** state);

// dump.c
void setUpTestDbForDump();
void testDumpWithDefaults(void** state);
void testDumpAsCsv(void** state);
void testDumpAbbrevUnits(void** state);
void testDumpFullUnits(void** state);

// data.c
void testAllocData(void** state);
void testMakeData(void** state);
void testFreeData(void** state);
void testAppendData(void** state);

// clientAlert.c
void testRemoveAlertsDb(void** state);
void testAddGetRemoveAlerts(void** state);
void testIsDateCriteriaPartMatch(void** state);
void testIsDateCriteriaMatch(void** state);
void testFindLowestMatch(void** state);
void testFindHighestMatchAtOrBelowLimit(void** state);
void testFindHighestMatch(void** state);
void testGetNonRelativeValue(void** state);
void testReplaceRelativeValues(void** state);
void testFindFirstMatchingDate(void** state);
void testGetTotalsForAlert(void** state);

// clientMonitor.c
void testMonitorEmptyDb(void** state);
void testMonitorNoDataAfterTs(void** state);
void testMonitorDataOnTs(void** state);
void testMonitorDataOnAndAfterTs(void** state);
void testMonitorDataOnAndLongAfterTs(void** state);
void testMonitorDataForFilter(void** state);
void testMonitorDataForAllFilters(void** state);

// clientQuery.c
void testQueryEmptyDb(void** state);
void testQueryNoDataInRange(void** state);
void testQueryNoDataForHost(void** state);
void testQueryDataInRangeHours(void** state);
void testQueryDataInRangeDays(void** state);
void testQueryDataInRangeMonths(void** state);
void testQueryDataInRangeYears(void** state);
void testQueryDataNarrowValueRangeSingleResult(void** state);
void testQueryDataNarrowValueRangeMultiResults(void** state);
void testQueryLargeQueryRange(void** state);

// clientSummary.c
void testSummaryEmptyDb(void** state);
void testSummaryOneEntry(void** state);
void testSummaryTwoEntriesSameTime(void** state);
void testSummaryTwoEntriesDifferentTimes(void** state);
void testSummaryEntriesSpanningDayBoundary(void** state);
void testSummaryEntriesSpanningMonthBoundary(void** state);
void testSummaryEntriesSpanningYearBoundary(void** state);
void testSummaryMultipleEntries(void** state);
void testSummaryOneOtherHost(void** state);
void testSummaryMultipleOtherHosts(void** state);

// clientSync.c
void testSyncEmptyDb(void** state);
void testSyncNoMatchingData(void** state);
void testSyncDataOnAndAfterTs(void** state);

// clientUtil.c
void testCalcTsBoundsEmptyDb(void** state);
void testCalcTsBoundsNoMatches(void** state);
void testCalcTsBoundsWithMatches(void** state);
void testGetValueForFilterIdNull(void** state);
void testGetValueForFilterIdNoMatch(void** state);
void testGetValueForFilterIdWithMatch(void** state);
void testCalcMaxValueEmptyDb(void** state);
void testCalcMaxValueWithData(void** state);
void testFormatAmountByUnits(void** state);

// config.c (bmdb)
void testConfigDump(void** state);
void testConfigUpdate(void** state);
void testConfigDelete(void** state);

// db.c
void testGetConfigInt(void** state);
void testGetConfigText(void** state);
void testSetConfigInt(void** state);
void testSetConfigText(void** state);
void testRmConfigBadValue(void** state);
void testRmConfigOkValue(void** state);
void testReadFiltersEmpty(void** state);
void testReadFiltersOk(void** state);
void testGetStmtSingleThreaded(void** state);
void testGetConfigPairsWithPrefixOk(void** state);
void testGetConfigPairsWithPrefixMissing(void** state);

// handleConfig.c
void testConfigWithAdmin(void** state);
void testConfigWithoutAdmin(void** state);
void testConfigUpdateWithoutAdmin(void** state);
void testConfigUpdateDisallowedParam(void** state);
void testConfigUpdateServerName(void** state);
void testConfigUpdateMonitorInterval(void** state);
void testConfigUpdateHistoryInterval(void** state);
void testConfigUpdateSummaryInterval(void** state);
void testConfigUpdateRssFreq(void** state);
void testConfigUpdateRssItems(void** state);
void testConfigUpdateDlColour(void** state);
void testConfigUpdateUlColour(void** state);

// time.c
void testTimeGm(void** state);
void testGetCurrentYearForTs(void** state);
void testGetCurrentMonthForTs(void** state);
void testGetCurrentDayForTs(void** state);
void testGetNextYearForTs(void** state);
void testGetNextMonthForTs(void** state);
void testGetNextDayForTs(void** state);
void testGetNextHourForTs(void** state);
void testGetNextMinForTs(void** state);
void testAddToDate(void** state);
void testNormaliseTm(void** state);

// process.c
void testProcess(void** state);

// sql.c
void testUpdateDbNull(void** state);
void testUpdateDbMultiple(void** state);
void testCompressSec1Filter(void** state);
void testCompressSecMultiFilters(void** state);
void testCompressSecMultiIterations(void** state);
void testCompressMin1Filter(void** state);
void testCompressMinMultiFilters(void** state);
void testGetNextCompressTime(void** state);

// bmclient/options.c
void testQueryMode(void** state);
void testSummaryMode(void** state);
void testDumpMode(void** state);
void testMonitorMode(void** state);
void testNoMode(void** state);

// bmsync/options.c
void testEmptyCmdLine(void** state);
void testPort(void** state);
void testVersion(void** state);
void testHelp(void** state);
void testHost(void** state);
void testAlias(void** state);
void testVariousValid(void** state);

// upgrade.c
void testUpgradeToCurrentLevel(void** state);
void testUpgradeToEarlierLevel(void** state);
void testUpgradeAboveMaxLevel(void** state);
void testUpgradeFrom1To2(void** state);
void testUpgradeFrom2To3(void** state);
void testUpgradeFrom3To4(void** state);
void testUpgradeFrom4To5(void** state);
void testUpgradeFrom5To6(void** state);
void testUpgradeFrom6To7(void** state);
void testConvertAddrValues(void** state);

// handleSummary.c
void testHandleSummary(void** state);

// handleFile.c
void testCharSubstitution(void** state);
void testGetMimeTypeForFile(void** state);

// httpRequest.c
void testParseRequest(void** state);
void testGetValueForName(void** state);
void testGetValueNumForName(void** state);

// handleQuery.c
void testMissingParam(void** state);
void testParamsOkOneFilter(void** state);
void testParamsOkMultiFilter(void** state);
void testGroupByDay(void** state);
void testParamsOkReversed(void** state);
void testGroupByDayCsv(void** state);

// handleMonitor
void testNoTsParam(void** state);
void testNoTgParam(void** state);
void testMonitorParamsOkOneFilter(void** state);
void testMonitorParamsOkMultiFilter(void** state);

// handleAlert.c
void testAlertNoAction(void** state);
void testAlertListNone(void** state);
void testAlertList(void** state);
void testAlertDeleteOk(void** state);
void testAlertDeleteForbidden(void** state);
void testAlertUpdateMissingArgs(void** state);
void testAlertUpdateOk(void** state);
void testAlertUpdateForbidden(void** state);
void testProcessAlertStatus(void** state);

// handleRss.c
void testRssHourlyNoAlerts(void** state);
void testRssWithAlertOk(void** state);
void testRssWithAlertExpired(void** state);
void testRssDailyNoAlerts(void** state);

// total.c
void testAllocTotal(void **state);
void testFreeTotals(void **state);
void testAppendTotals(void **state);

// handleSync.c
void setupTestForHandleSync(void** state);
void tearDownTestForHandleSync(void** state);
void testSyncNoTsParam(void** state);
void testSyncTsParamOk(void** state);

// clientFilter.c
void testGetFilter(void** state);
void testReadFilters(void** state);
void testFilterExprIsValid(void** state);
void testAddFilter(void** state);
void testRemoveFilter(void** state);
void testReadFiltersForHost(void** state);