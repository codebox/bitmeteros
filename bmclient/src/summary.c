#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <sqlite3.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"
#include "common.h"

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

        PRINT(COLOUR_WHITE_2, "%*s       Today      Month       Year      Total\n", filterColWidth, "");
        PRINT(COLOUR_WHITE_2, "%*s  ---------- ---------- ---------- ----------\n", filterColWidth, "");
        
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
            
            PRINT(COLOUR_WHITE_2, "%*s:", filterColWidth, filter->desc);
            PRINT(COLOUR_WHITE_1, " %10s %10s %10s %10s\n", todayVal, monthVal, yearVal, fullVal);
            filter = filter->next;
        }
        
        printf("\n");
        PRINT(COLOUR_WHITE_2, "Data recorded from: ");
        PRINT(COLOUR_WHITE_1, "%s %s\n", minTsDate, minTsTime);
        PRINT(COLOUR_WHITE_2, "Data recorded to:   ");
        PRINT(COLOUR_WHITE_1, "%s %s\n", maxTsDate, maxTsTime);
        printf("\n");

        if (summary.hostCount == 0){
            PRINT(COLOUR_WHITE_2, "No data for other hosts.\n");
        } else {
            PRINT(COLOUR_WHITE_2, "Totals include data for %d other host%s:\n", summary.hostCount, (summary.hostCount == 1) ? "" : "s");
            int i;
            for (i=0; i<summary.hostCount; i++){
                PRINT(COLOUR_WHITE_1, "    %s\n", summary.hostNames[i]);
            }
        }
        printf("\n");
    } else {
        printf("        The database is empty.\n\n");
    }

    char dbPath[MAX_PATH_LEN];
    getDbPath(dbPath);
    PRINT(COLOUR_WHITE_2, "Database location:  ");
    PRINT(COLOUR_WHITE_1, "%s\n", dbPath);
}
