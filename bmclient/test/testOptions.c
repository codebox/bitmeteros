#define _GNU_SOURCE
#include <stdlib.h>
#include "common.h"
#include "string.h"
#include "test.h"
#include "client.h"
#include "bmclient.h"
#include "CuTest.h"

extern optind;
static void checkPrefs(CuTest *tc, struct Prefs expectedPrefs, char* cmdLine);

void checkQueryRangeOk(CuTest *tc, char* cmdLine, time_t from, time_t to){
	struct Prefs prefs = {PREF_MODE_QUERY, 0, 0, 0, 0, from, to, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs, cmdLine);
}
void checkQueryRangeErr(CuTest *tc, char* cmdLine){
	struct Prefs prefs = {PREF_MODE_QUERY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_RANGE};
	checkPrefs(tc, prefs, cmdLine);
}

void testQueryMode(CuTest *tc) {
	struct Prefs prefs1 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_HOURS, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs1, "-mq -r2009 -gh");

	struct Prefs prefs2 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_DAYS, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs2, "-mq -r2009 -gd");

	struct Prefs prefs3 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_MONTHS, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs3, "-mq -r2009 -gm");

	struct Prefs prefs4 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_YEARS, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs4, "-mq -r2009 -gy");

	struct Prefs prefs5 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_TOTAL, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs5, "-mq -r2009 -gt");

	struct Prefs prefs6 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, 0, ERR_OPT_BAD_GROUP};
	checkPrefs(tc, prefs6, "-mq -r2009 -gz");

	struct Prefs prefs7 = {PREF_MODE_QUERY, 0, PREF_UNITS_BYTES, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs7, "-mq -r2009 -ub");

	struct Prefs prefs8 = {PREF_MODE_QUERY, 0, PREF_UNITS_ABBREV, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs8, "-mq -r2009 -ua");

	struct Prefs prefs9 = {PREF_MODE_QUERY, 0, PREF_UNITS_FULL, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs9, "-mq -r2009 -uf");

	struct Prefs prefs10 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, 0, ERR_OPT_BAD_UNIT};
	checkPrefs(tc, prefs10, "-mq -r2009 -uz");

	struct Prefs prefs11 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs11, "-mq -r2009");

	//checkQueryRangeErr(tc, "-mq");
	checkQueryRangeErr(tc, "-mq -rz");
	checkQueryRangeErr(tc, "-mq -r200");
	checkQueryRangeErr(tc, "-mq -r200x");
	checkQueryRangeErr(tc, "-mq -r1969123123");
	checkQueryRangeErr(tc, "-mq -r20091");
	//checkQueryRangeErr(tc, "-mq -r200900");
	//checkQueryRangeErr(tc, "-mq -r200913");
	checkQueryRangeErr(tc, "-mq -r2009123");
	//checkQueryRangeErr(tc, "-mq -r20090100");
	//checkQueryRangeErr(tc, "-mq -r20090132");
	//checkQueryRangeErr(tc, "-mq -r20090229");

	checkQueryRangeErr(tc, "-mq -r2009-");
	checkQueryRangeErr(tc, "-mq -r2009-x");
	checkQueryRangeErr(tc, "-mq -r2009-1960");
	//checkQueryRangeErr(tc, "-mq -r200901-200913");
	//checkQueryRangeErr(tc, "-mq -r20090112-20090132");
	//checkQueryRangeErr(tc, "-mq -r2009011223-2009011225");
	checkQueryRangeErr(tc, "-mq -r2009011222-20090112239");

	checkQueryRangeOk(tc, "-mq -r1970", 0, 31536000);
	checkQueryRangeOk(tc, "-mq -r1970-1970", 0, 31536000);
	checkQueryRangeOk(tc, "-mq -r1970-2008", 0, 1230768000);
	checkQueryRangeOk(tc, "-mq -r2008-1970", 0, 1230768000);

	checkQueryRangeOk(tc, "-mq -r197001", 0, 2678400);
	checkQueryRangeOk(tc, "-mq -r197001-197001", 0, 2678400);
	checkQueryRangeOk(tc, "-mq -r197001-200812", 0, 1230768000);
	checkQueryRangeOk(tc, "-mq -r200812-197001", 0, 1230768000);

	checkQueryRangeOk(tc, "-mq -r19700101", 0, 86400);
	checkQueryRangeOk(tc, "-mq -r19700101-19700101", 0, 86400);
	checkQueryRangeOk(tc, "-mq -r19700101-20021231", 0, 1041379200);
	checkQueryRangeOk(tc, "-mq -r20021231-19700101", 0, 1041379200);

	checkQueryRangeOk(tc, "-mq -r1970010100", 0, 3600);
	checkQueryRangeOk(tc, "-mq -r1970010100-1970010100", 0, 3600);
	checkQueryRangeOk(tc, "-mq -r1970010100-2008123123", 0, 1230768000);
	checkQueryRangeOk(tc, "-mq -r2008123123-1970010100", 0, 1230768000);

	checkQueryRangeOk(tc, "-mq -r2008-2001011119", 979239600, 1230768000);
	checkQueryRangeOk(tc, "-mq -r200807-20090101", 1214870400, 1230854400);
}

void testSummaryMode(CuTest *tc) {
	struct Prefs prefs = {PREF_MODE_SUMMARY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs, "-ms");

}
void testDumpMode(CuTest *tc) {
	struct Prefs prefs1 = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs1, "-md");

	struct Prefs prefs2 = {PREF_MODE_DUMP, PREF_DUMP_FORMAT_CSV, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs2, "-md -fc");

	struct Prefs prefs3 = {PREF_MODE_DUMP, PREF_DUMP_FORMAT_FIXED_WIDTH, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs3, "-md -ff");

	struct Prefs prefs4 = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_DUMP_FORMAT};
	checkPrefs(tc, prefs4, "-md -fz");

	struct Prefs prefs5 = {PREF_MODE_DUMP, 0, PREF_UNITS_BYTES, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs5, "-md -ub");

	struct Prefs prefs6 = {PREF_MODE_DUMP, 0, PREF_UNITS_ABBREV, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs6, "-md -ua");

	struct Prefs prefs7 = {PREF_MODE_DUMP, 0, PREF_UNITS_FULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs7, "-md -uf");

	struct Prefs prefs8 = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_UNIT};
	checkPrefs(tc, prefs8, "-md -uz");
}

void testNoMode(CuTest *tc) {
	struct Prefs prefs1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_NO_ARGS};
	checkPrefs(tc, prefs1, "");

	struct Prefs prefs2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_MODE};
	checkPrefs(tc, prefs2, "-mz");

	struct Prefs prefs3 = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs3, "-v");

	struct Prefs prefs4 = {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs4, "-h");
}

void testMonitorMode(CuTest *tc) {
	struct Prefs prefs1 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	checkPrefs(tc, prefs1, "-mm");

	struct Prefs prefs2 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PREF_MONITOR_TYPE_NUMS, NULL};
	checkPrefs(tc, prefs2, "-mm -tn");

	struct Prefs prefs3 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PREF_MONITOR_TYPE_BAR, NULL};
	checkPrefs(tc, prefs3, "-mm -tb");

	struct Prefs prefs4 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_MONITOR_TYPE};
	checkPrefs(tc, prefs4, "-mm -tz");

	struct Prefs prefs5 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, PREF_DIRECTION_DL, 0, 0, 0, NULL};
	checkPrefs(tc, prefs5, "-mm -dd");

	struct Prefs prefs6 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, PREF_DIRECTION_UL, 0, 0, 0, NULL};
	checkPrefs(tc, prefs6, "-mm -du");

	struct Prefs prefs7 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_DIRECTION};
	checkPrefs(tc, prefs7, "-mm -dz");

	struct Prefs prefs8 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, NULL};
	checkPrefs(tc, prefs8, "-mm -w10");

	struct Prefs prefs9 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_WIDTH};
	checkPrefs(tc, prefs9, "-mm -w0");

	struct Prefs prefs10 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_WIDTH};
	checkPrefs(tc, prefs10, "-mm -wz");

	struct Prefs prefs11 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1000000, 0, NULL};
	checkPrefs(tc, prefs11, "-mm -x1000000");

	struct Prefs prefs12 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_MAX};
	checkPrefs(tc, prefs12, "-mm -x0");

	struct Prefs prefs13 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ERR_OPT_BAD_MAX};
	checkPrefs(tc, prefs13, "-mm -xz");
}

static void checkPrefs(CuTest *tc, struct Prefs expectedPrefs, char* cmdLine){
	int argc = 1;

	char *cmdLineCopy = strdupa(cmdLine);
	char* match = strtok(cmdLineCopy, " ");
	while(match != NULL){
        argc++;
        match = strtok(NULL, " ");
	}

    char* argv[argc];
    argv[0] = NULL;

    int i=1;
    cmdLineCopy = strdupa(cmdLine);
    match = strtok(cmdLineCopy, " ");
	do{
        argv[i++] = match;
        match = strtok(NULL, " ");
	} while(match != NULL);

	struct Prefs actualPrefs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	optind = 1; // need to reset this global between each call to getopt()
	parseArgs(argc, argv, &actualPrefs);

	CuAssertIntEquals(tc, expectedPrefs.mode,        actualPrefs.mode);
	CuAssertIntEquals(tc, expectedPrefs.dumpFormat,  actualPrefs.dumpFormat);
	CuAssertIntEquals(tc, expectedPrefs.units,       actualPrefs.units);
	CuAssertIntEquals(tc, expectedPrefs.help,        actualPrefs.help);
	CuAssertIntEquals(tc, expectedPrefs.version,     actualPrefs.version);
	CuAssertIntEquals(tc, expectedPrefs.rangeFrom,   actualPrefs.rangeFrom);
	CuAssertIntEquals(tc, expectedPrefs.rangeTo,     actualPrefs.rangeTo);
	CuAssertIntEquals(tc, expectedPrefs.group,       actualPrefs.group);
	CuAssertIntEquals(tc, expectedPrefs.direction,   actualPrefs.direction);
	CuAssertIntEquals(tc, expectedPrefs.barChars,    actualPrefs.barChars);
	CuAssertIntEquals(tc, expectedPrefs.maxAmount,   actualPrefs.maxAmount);
	CuAssertIntEquals(tc, expectedPrefs.monitorType, actualPrefs.monitorType);

	if (expectedPrefs.errorMsg == NULL){
		CuAssertTrue(tc, actualPrefs.errorMsg == NULL);
	} else {
		CuAssertStrEquals(tc, expectedPrefs.errorMsg, actualPrefs.errorMsg);
	}
}

CuSuite* optionsGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testQueryMode);
    SUITE_ADD_TEST(suite, testSummaryMode);
    SUITE_ADD_TEST(suite, testDumpMode);
    SUITE_ADD_TEST(suite, testMonitorMode);
    SUITE_ADD_TEST(suite, testNoMode);
    return suite;
}
