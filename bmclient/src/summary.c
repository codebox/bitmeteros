#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <sqlite3.h>
#include "common.h"
#include "bmclient.h"

extern struct Prefs prefs;
static void calcSummary();
static void printSummary();

static sqlite3_stmt *stmtTotalsSince;

struct BwValues todaysTotal, monthsTotals, yearTotals, fullTotals;
struct ValuesBounds tsBounds;

static struct BwValues calcTotalsSince(int);

void doSummary(){
	prepareSql(&stmtTotalsSince, "select sum(dl), sum(ul) from data where ts>=?");

	calcSummary();
	printSummary();
}

static void calcSummary(){
	int now = getTime();

	int tsForStartOfToday = getCurrentDayForTs(now);
	todaysTotal  = calcTotalsSince(tsForStartOfToday);

	int tsForStartOfMonth = getCurrentMonthForTs(now);
	monthsTotals = calcTotalsSince(tsForStartOfMonth);

	int tsForStartOfYear = getCurrentYearForTs(now);
	yearTotals = calcTotalsSince(tsForStartOfYear);

	fullTotals = calcTotalsSince(0);

	tsBounds = calcTsBounds();
}

static struct BwValues calcTotalsSince(int ts){
	sqlite3_bind_int(stmtTotalsSince, 1, ts);

	unsigned long long dlTotal, ulTotal;
	int rc = sqlite3_step(stmtTotalsSince);
	if (rc == SQLITE_ROW){
		dlTotal = sqlite3_column_int64(stmtTotalsSince, 0);
		ulTotal = sqlite3_column_int64(stmtTotalsSince, 1);
	} else {
		dlTotal = ulTotal = 0;
	}
	sqlite3_reset(stmtTotalsSince);

	struct BwValues values;
	values.dl = dlTotal;
	values.ul = ulTotal;
	return values;
}

static void printSummary(){
	char* todayDl = (char*) calloc(20, sizeof(char));
	char* todayUl = (char*) calloc(20, sizeof(char));
	char* monthDl = (char*) calloc(20, sizeof(char));
	char* monthUl = (char*) calloc(20, sizeof(char));
	char* yearDl  = (char*) calloc(20, sizeof(char));
	char* yearUl  = (char*) calloc(20, sizeof(char));
	char* fullDl  = (char*) calloc(20, sizeof(char));
	char* fullUl  = (char*) calloc(20, sizeof(char));
	char* minTsDate = (char*) calloc(11, sizeof(char));
	char* minTsTime = (char*) calloc(9, sizeof(char));
	char* maxTsDate = (char*) calloc(11, sizeof(char));
	char* maxTsTime = (char*) calloc(9, sizeof(char));

	formatAmount(todaysTotal.dl,  0, PREF_UNITS_ABBREV, todayDl);
	formatAmount(todaysTotal.ul,  0, PREF_UNITS_ABBREV, todayUl);
	formatAmount(monthsTotals.dl, 0, PREF_UNITS_ABBREV, monthDl);
	formatAmount(monthsTotals.ul, 0, PREF_UNITS_ABBREV, monthUl);
	formatAmount(yearTotals.dl,   0, PREF_UNITS_ABBREV, yearDl);
	formatAmount(yearTotals.ul,   0, PREF_UNITS_ABBREV, yearUl);
	formatAmount(fullTotals.dl,   0, PREF_UNITS_ABBREV, fullDl);
	formatAmount(fullTotals.ul,   0, PREF_UNITS_ABBREV, fullUl);

	toDate(minTsDate, tsBounds.min);
	toTime(minTsTime, tsBounds.min);
	toDate(maxTsDate, tsBounds.max);
	toTime(maxTsTime, tsBounds.max);

	printf("Summary\n");
	printf("                DL          UL\n");
	printf("        ----------  ----------\n");
	printf("Today:  %10s  %10s\n",   todayDl, todayUl);
	printf("Month:  %10s  %10s\n", monthDl, monthUl);
	printf("Year:   %10s  %10s\n", yearDl,  yearUl);
	printf("Total:  %10s  %10s\n", fullDl,  fullUl);
	printf("\n");
	printf("Data recorded from: %s %s\n", minTsDate, minTsTime);
	printf("Data recorded to:   %s %s\n", maxTsDate, maxTsTime);
	printf("\n");

	char dbPath[MAX_PATH_LEN];
	getDbPath(dbPath);
	printf("Database location:  %s\n", dbPath);
}
