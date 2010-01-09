/*
 * BitMeterOS v0.3.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

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
	char todayDl[20];
	char todayUl[20];
	char monthDl[20];
	char monthUl[20];
	char yearDl[20];
	char yearUl[20];
	char fullDl[20];
	char fullUl[20];
	char minTsDate[11];
	char minTsTime[9];
	char maxTsDate[11];
	char maxTsTime[9];

    int dbEmpty = (summary.tsMax == 0);

    printf("Summary\n");

    if (!dbEmpty){
     // Todays totals
        formatAmount(summary.today->dl,  TRUE, PREF_UNITS_ABBREV, todayDl);
        formatAmount(summary.today->ul,  TRUE, PREF_UNITS_ABBREV, todayUl);

     // This months totals
        formatAmount(summary.month->dl,  TRUE, PREF_UNITS_ABBREV, monthDl);
        formatAmount(summary.month->ul,  TRUE, PREF_UNITS_ABBREV, monthUl);

     // This years totals
        formatAmount(summary.year->dl,   TRUE, PREF_UNITS_ABBREV, yearDl);
        formatAmount(summary.year->ul,   TRUE, PREF_UNITS_ABBREV, yearUl);

     // Grand totals
        formatAmount(summary.total->dl, TRUE, PREF_UNITS_ABBREV, fullDl);
        formatAmount(summary.total->ul, TRUE, PREF_UNITS_ABBREV, fullUl);

        toDate(minTsDate, summary.tsMin);
        toTime(minTsTime, summary.tsMin);
        toDate(maxTsDate, summary.tsMax);
        toTime(maxTsTime, summary.tsMax);

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
