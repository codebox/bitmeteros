#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <sqlite3.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"

/*
Contains the code that handles summary requests made via the bmclient utility.
*/

extern struct Prefs prefs;
static void printSummary();
struct ValueBounds tsBounds;

void doSummary(){
 // Compute the summary data
	struct Summary summary = getSummaryValues();

 // Display the summary
	printSummary(summary);

 // Free memory
	freeSummary(&summary);
}

static void printSummary(struct Summary summary){
	char minTsDate[11];
	char minTsTime[9];
	char maxTsDate[11];
	char maxTsTime[9];

    int noData = (summary.tsMax == 0);

    printf("Summary\n");

    if (!noData){
        toDate(minTsDate, summary.tsMin);
        toTime(minTsTime, summary.tsMin);
        toDate(maxTsDate, summary.tsMax);
        toTime(maxTsTime, summary.tsMax);
	
		struct Filter* filter = readFilters();
		int filterColWidth = getMaxFilterDescWidth(filter);

        printf("%*s       Today      Month       Year      Total\n", filterColWidth, "");
        printf("%*s  ---------- ---------- ---------- ----------\n", filterColWidth, "");
        
		while (filter != NULL){
			char todayVal[20];
			char monthVal[20];
			char yearVal[20];
			char fullVal[20];
			
			BW_INT value;
			
			value = getValueForFilterId(summary.today, filter->id);
			formatAmount(value, TRUE, PREF_UNITS_ABBREV, todayVal);
			
			value = getValueForFilterId(summary.month, filter->id);
			formatAmount(value, TRUE, PREF_UNITS_ABBREV, monthVal);
			
			value = getValueForFilterId(summary.year, filter->id);
			formatAmount(value,  TRUE, PREF_UNITS_ABBREV, yearVal);
			
			value = getValueForFilterId(summary.total, filter->id);
			formatAmount(value, TRUE, PREF_UNITS_ABBREV, fullVal);
			
			printf("%*s: %10s %10s %10s %10s\n", filterColWidth, filter->desc, todayVal, monthVal, yearVal, fullVal);
			filter = filter->next;
		}
        
        printf("\n");
        printf("Data recorded from: %s %s\n", minTsDate, minTsTime);
        printf("Data recorded to:   %s %s\n", maxTsDate, maxTsTime);
        printf("\n");

        if (summary.hostCount == 0){
            printf("No data for other hosts.\n");
        } else {
            printf("Totals include data for %d other host%s:\n", summary.hostCount, (summary.hostCount == 1) ? "" : "s");
            int i;
            for (i=0; i<summary.hostCount; i++){
                printf("    %s\n", summary.hostNames[i]);
            }
        }
        printf("\n");
    } else {
        printf("        The database is empty.\n\n");
    }

	char dbPath[MAX_PATH_LEN];
	getDbPath(dbPath);
	printf("Database location:  %s\n", dbPath);
}
