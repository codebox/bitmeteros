#include <stdlib.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"

int main(int argc, char* argv[]) { 
    const UnitTest tests[] = { 
        unit_test(testAllocFilter),
        unit_test(testCopyFilter),
        unit_test(testFilterHasHost),
        unit_test(testAllocFilterWithNulls),
        unit_test(testFreeFilter),
        unit_test(testGetFilterFromId),
        unit_test(testGetFilterFromName),
        unit_test(testAppendFilter),
        unit_test(testGetTotalForFilter),
        unit_test(testGetMaxFilterDescWidth),
        unit_test(testGetMaxFilterNameWidth),
        unit_test(testFilterNameIsValid),
        unit_test(testAllocAdapter),
        unit_test(testFreeAdapters),
        unit_test(testAppendAdapter),
        unit_test(testAllocAlert),
        unit_test(testSetAlertName),
        unit_test(testAppendAlert),
        unit_test(testMakeDateCriteriaPart),
        unit_test(testDateCriteriaPartToText),
        unit_test(testMakeDateCriteria),
        unit_test(testAppendDateCriteria),
        unit_test(testFormatAmounts),
        unit_test(testToTime),
        unit_test(testToDate),
        unit_test(testMakeHexString),
        unit_test(testStrToLong),
        unit_test(testReplace),
        unit_test_setup_teardown(testClientDumpEmptyDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testClientDumpOneEntry, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testClientDumpMultipleEntries, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainHelp, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainVersion, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainFailWithError, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainFailNoError, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainDoDump, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainDoSummary, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainDoQuery, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMainDoMonitor, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testDumpAbbrevUnits,  setUpTestDbForDump, tearDownTestDb),
        unit_test_setup_teardown(testDumpAsCsv,        setUpTestDbForDump, tearDownTestDb),
        unit_test_setup_teardown(testDumpWithDefaults, setUpTestDbForDump, tearDownTestDb),
        unit_test_setup_teardown(testDumpFullUnits,    setUpTestDbForDump, tearDownTestDb),
        unit_test(testAllocData),
        unit_test(testMakeData),
        unit_test(testFreeData),
        unit_test(testAppendData),
        unit_test_setup_teardown(testRemoveAlertsDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAddGetRemoveAlerts, setupTestDb, tearDownTestDb),
        unit_test(testIsDateCriteriaPartMatch),
        unit_test(testIsDateCriteriaMatch),
        unit_test(testFindLowestMatch),
        unit_test(testFindHighestMatchAtOrBelowLimit),
        unit_test(testFindHighestMatch),
        unit_test(testGetNonRelativeValue),
        unit_test(testReplaceRelativeValues),
        unit_test(testFindFirstMatchingDate),
        unit_test_setup_teardown(testGetTotalsForAlert, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorEmptyDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorNoDataAfterTs, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorDataOnTs, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorDataOnAndAfterTs, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorDataOnAndLongAfterTs, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorDataForFilter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorDataForAllFilters, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryEmptyDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryNoDataInRange, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryNoDataForHost, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryDataInRangeHours, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryDataInRangeDays, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryDataInRangeMonths, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryDataInRangeYears, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryDataNarrowValueRangeSingleResult, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryDataNarrowValueRangeMultiResults, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testQueryLargeQueryRange, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryEmptyDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryOneEntry, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryTwoEntriesSameTime, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryTwoEntriesDifferentTimes, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryEntriesSpanningDayBoundary, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryEntriesSpanningMonthBoundary, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryEntriesSpanningYearBoundary, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryMultipleEntries, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryOneOtherHost, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSummaryMultipleOtherHosts, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSyncEmptyDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSyncNoMatchingData, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSyncDataOnAndAfterTs, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCalcTsBoundsEmptyDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCalcTsBoundsNoMatches, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCalcTsBoundsWithMatches, setupTestDb, tearDownTestDb),
        unit_test(testGetValueForFilterIdNull),
        unit_test(testGetValueForFilterIdNoMatch),
        unit_test(testGetValueForFilterIdWithMatch),
        unit_test_setup_teardown(testCalcMaxValueEmptyDb, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCalcMaxValueWithData, setupTestDb, tearDownTestDb),
        unit_test(testFormatAmountByUnits),
        unit_test_setup_teardown(testConfigDump, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdate, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigDelete, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetConfigInt, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetConfigText, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSetConfigInt, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSetConfigText, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetConfigPairsWithPrefixOk, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetConfigPairsWithPrefixMissing, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testRmConfigBadValue, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testRmConfigOkValue, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testReadFiltersEmpty, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testReadFiltersOk, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetStmtSingleThreaded, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigWithAdmin, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigWithoutAdmin, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateWithoutAdmin, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateDisallowedParam, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateServerName, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateMonitorInterval, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateHistoryInterval, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateSummaryInterval, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateRssFreq, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConfigUpdateRssItems, setupTestDb, tearDownTestDb),
        unit_test(testTimeGm),
        unit_test(testGetCurrentYearForTs),
        unit_test(testGetCurrentMonthForTs),
        unit_test(testGetCurrentDayForTs),
        unit_test(testGetNextYearForTs),
        unit_test(testGetNextMonthForTs),
        unit_test(testGetNextDayForTs),
        unit_test(testGetNextHourForTs),
        unit_test(testGetNextMinForTs),
        unit_test(testAddToDate),
        unit_test(testNormaliseTm),
        unit_test(testProcess),
        unit_test_setup_teardown(testUpdateDbNull, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpdateDbMultiple, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCompressSec1Filter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCompressSecMultiFilters, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCompressSecMultiIterations, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCompressMin1Filter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCompressMinMultiFilters, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetNextCompressTime, setupTestDb, tearDownTestDb),
        unit_test(testQueryMode),
        unit_test(testSummaryMode),
        unit_test(testDumpMode),
        unit_test(testMonitorMode),
        unit_test(testNoMode),
        unit_test(testEmptyCmdLine),
        unit_test(testPort),
        unit_test(testVersion),
        unit_test(testHelp),
        unit_test(testHost),
        unit_test(testAlias),
        unit_test(testVariousValid),
        unit_test_setup_teardown(testUpgradeToCurrentLevel, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeToEarlierLevel, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeAboveMaxLevel, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeFrom1To2, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeFrom2To3, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeFrom3To4, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeFrom4To5, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeFrom5To6, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeFrom6To7, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testUpgradeFrom7To8, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testConvertAddrValues, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testHandleSummary, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testCharSubstitution, setupTestDb, tearDownTestDb),
        unit_test(testGetMimeTypeForFile),
        unit_test(testParseRequest),
        unit_test(testGetValueForName),
        unit_test(testGetValueNumForName),
        unit_test_setup_teardown(testMissingParam, setupTestForHandleQuery, tearDownTestDb),
        unit_test_setup_teardown(testParamsOkOneFilter, setupTestForHandleQuery, tearDownTestDb),
        unit_test_setup_teardown(testParamsOkMultiFilter, setupTestForHandleQuery, tearDownTestDb),
        unit_test_setup_teardown(testGroupByDay, setupTestForHandleQuery, tearDownTestDb),
        unit_test_setup_teardown(testParamsOkReversed, setupTestForHandleQuery, tearDownTestDb),
        unit_test_setup_teardown(testGroupByDayCsv, setupTestForHandleQuery, tearDownTestDb),
        unit_test_setup_teardown(testNoTsParam, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testNoTgParam, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorParamsOkOneFilter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testMonitorParamsOkMultiFilter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertNoAction, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertListNone, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertList, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertDeleteOk, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertDeleteForbidden, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertUpdateMissingArgs, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertUpdateOk, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAlertUpdateForbidden, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testProcessAlertStatus, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testRssHourlyNoAlerts, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testRssWithAlertOk, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testRssWithAlertExpired, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testRssDailyNoAlerts, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAllocTotal, setupTestForTotal, teardownTestForTotal),
        unit_test_setup_teardown(testFreeTotals, setupTestForTotal, teardownTestForTotal),
        unit_test_setup_teardown(testAppendTotals, setupTestForTotal, teardownTestForTotal),
        unit_test_setup_teardown(testSyncNoTsParam, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testSyncTsParamOk, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetFilter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testReadFilters, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testAddFilter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testRemoveFilter, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testFilterExprIsValid, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testReadFiltersForHost, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testGetMaxTsForHost, setupTestDb, tearDownTestDb),
        unit_test(testParseFilterRow),
        unit_test(testParseDataRow),
        unit_test(testStartsWith),
        unit_test(testGetLocalId),
        unit_test_setup_teardown(testGetLocalFilter, setupTestDb, tearDownTestDb),
        unit_test(testAppendRemoteFilter),
        unit_test_setup_teardown(testRemoveDataForDeletedFiltersFromThisHost, setupTestDb, tearDownTestDb),
        unit_test(testReadLine),
        unit_test_setup_teardown(testHttpHeadersOk, setupTestDb, tearDownTestDb),
        unit_test_setup_teardown(testParseDataOk, setupTestDb, tearDownTestDb),
        unit_test(testSendReqToDefaultPort),
        unit_test(testSendReqToOtherPort),
        unit_test(testStrAppend),
        unit_test_setup_teardown(testBuildFilterPairs, setupTestDb, tearDownTestDb),
        unit_test(testFreeNameValuePairs),
        unit_test(testAppendNameValuePair),
        unit_test(testMakeHtmlFromData)

    }; 
    return run_tests(tests); 
}
