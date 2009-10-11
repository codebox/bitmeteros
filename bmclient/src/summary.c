#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <sqlite3.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"

extern struct Prefs prefs;
static void printSummary();

struct ValueBounds tsBounds;

void doSummary(){
 // Compute the summary data
	struct Summary summary = getSummaryValues();

 // Display the summary
	printSummary(summary);

 // Free memory
	freeData(summary.today);
	freeData(summary.month);
	freeData(summary.year);
	freeData(summary.total);
}

static void printSummary(struct Summary summary){
	char* todayDl   = (char*) calloc(20, sizeof(char));
	char* todayUl   = (char*) calloc(20, sizeof(char));
	char* monthDl   = (char*) calloc(20, sizeof(char));
	char* monthUl   = (char*) calloc(20, sizeof(char));
	char* yearDl    = (char*) calloc(20, sizeof(char));
	char* yearUl    = (char*) calloc(20, sizeof(char));
	char* fullDl    = (char*) calloc(20, sizeof(char));
	char* fullUl    = (char*) calloc(20, sizeof(char));
	char* minTsDate = (char*) calloc(11, sizeof(char));
	char* minTsTime = (char*) calloc(9,  sizeof(char));
	char* maxTsDate = (char*) calloc(11, sizeof(char));
	char* maxTsTime = (char*) calloc(9,  sizeof(char));

 // Todays totals
	formatAmount(summary.today->dl,  0, PREF_UNITS_ABBREV, todayDl);
	formatAmount(summary.today->ul,  0, PREF_UNITS_ABBREV, todayUl);
	
 // This months totals
	formatAmount(summary.month->dl,  0, PREF_UNITS_ABBREV, monthDl);
	formatAmount(summary.month->ul,  0, PREF_UNITS_ABBREV, monthUl);
	
 // This years totals
	formatAmount(summary.year->dl,   0, PREF_UNITS_ABBREV, yearDl);
	formatAmount(summary.year->ul,   0, PREF_UNITS_ABBREV, yearUl);
	
 // Grand totals
	formatAmount(summary.total->dl, 0, PREF_UNITS_ABBREV, fullDl);
	formatAmount(summary.total->ul, 0, PREF_UNITS_ABBREV, fullUl);

	toDate(minTsDate, summary.tsMin);
	toTime(minTsTime, summary.tsMin);
	toDate(maxTsDate, summary.tsMax);
	toTime(maxTsTime, summary.tsMax);

	printf("Summary\n");
	printf("                DL          UL\n");
	printf("        ----------  ----------\n");
	printf("Today:  %10s  %10s\n", todayDl, todayUl);
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
