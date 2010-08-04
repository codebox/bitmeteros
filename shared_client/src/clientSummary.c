/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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
 */

#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "client.h"

/*
Contains a helper function for use by clients that need to produce database summaries.
*/

#define DATA_SUMMARY_SQL_ALL          "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>=?"
#define DATA_SUMMARY_SQL_HOST         "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>=? AND hs=?"
#define DATA_SUMMARY_SQL_HOST_ADAPTER "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>=? AND hs=? AND ad=?"
#define HOST_SUMMARY_SQL              "SELECT DISTINCT(hs) AS hs FROM data WHERE hs != ''"

struct Summary getSummaryValues(char* hs, char* ad);
static void getHosts(char*** hostNames, int* hostCount);

struct Summary getSummaryValues(char* hs, char* ad){
    int selectByAdapter = (ad == NULL ? FALSE : TRUE);
    int selectByHost    = (hs == NULL ? FALSE : TRUE);

 // Populate a Summary struct
	struct Summary summary;
	int now = getTime();

	int tsForStartOfToday = getCurrentDayForTs(now);
	int tsForStartOfMonth = getCurrentMonthForTs(now);
	int tsForStartOfYear = getCurrentYearForTs(now);

    struct Data* (*calcTotals)(int, char*, char*);

    if (selectByAdapter == TRUE) {
        if (selectByHost == TRUE) {
         // We want data only for 1 specific adapter on 1 host
            calcTotals = &calcTotalsForHsAdSince;
            summary.hostNames = NULL;
            summary.hostCount = 0;

        } else {
         // This doesn't really make sense
            logMsg(LOG_WARN, "getSummaryValues called with null 'hs' but a non-null 'ad' of %s", ad);
            calcTotals = NULL;
        }

    } else {
        if (selectByHost == TRUE) {
         // We want data only for 1 specific host
            calcTotals = &calcTotalsForHsSince;
            summary.hostNames = NULL;
            summary.hostCount = 0;

        } else {
         // We want all data regardless of where it is from
            calcTotals = &calcTotalsForAllSince;
            getHosts(&(summary.hostNames), &(summary.hostCount));
        }
    }

	summary.today = calcTotals(tsForStartOfToday, hs, ad);
	summary.month = calcTotals(tsForStartOfMonth, hs, ad);
	summary.year  = calcTotals(tsForStartOfYear, hs, ad);
	summary.total = calcTotals(0, hs, ad);

	struct ValueBounds* tsBounds = calcTsBounds(hs, ad);
	if (tsBounds != NULL){
        summary.tsMin = tsBounds->min;
        summary.tsMax = tsBounds->max;
        free(tsBounds);
	} else {
	 // No data found for host/adapter
        summary.tsMin = 0;
        summary.tsMax = 0;
	}

	return summary;
}

void freeSummary(struct Summary* summary){
 // Release all the memory allocated to this struct
    freeData(summary->today);
	freeData(summary->month);
	freeData(summary->year);
	freeData(summary->total);

    int i;
    for(i=0; i<summary->hostCount; i++){
        free(summary->hostNames[i]);
    }
}


static void getHosts(char*** hostNames, int* hostCount){
    sqlite3_stmt *stmt = getStmt(HOST_SUMMARY_SQL);

    struct Data* data = runSelect(stmt);
    struct Data* thisData;

    *hostCount = 0;
    thisData = data;
    while(thisData != NULL){
        (*hostCount)++;
        thisData = thisData->next;
    }

    *hostNames = malloc(sizeof(char*) * (*hostCount));

    thisData = data;
    int offset = 0;
    while(thisData != NULL){
        (*hostNames)[offset++] = strdup(thisData->hs);
        thisData = thisData->next;
    }
    
	freeData(data);
    finishedStmt(stmt);
}

struct Data* calcTotalsForAllSince(int ts, char* hs, char* ad){
    sqlite3_stmt *stmt = getStmt(DATA_SUMMARY_SQL_ALL);

	sqlite3_bind_int(stmt, 1, ts);
	struct Data* result = runSelect(stmt);

	finishedStmt(stmt);

	return result;
}

struct Data* calcTotalsForHsSince(int ts, char* hs, char* ad){
    sqlite3_stmt *stmt = getStmt(DATA_SUMMARY_SQL_HOST);

	sqlite3_bind_int(stmt, 1, ts);
    sqlite3_bind_text(stmt, 2, hs, strlen(hs), SQLITE_TRANSIENT);
	struct Data* data = runSelect(stmt);

	finishedStmt(stmt);

	return data;
}

struct Data* calcTotalsForHsAdSince(int ts, char* hs, char* ad){
    sqlite3_stmt *stmt = getStmt(DATA_SUMMARY_SQL_HOST_ADAPTER);

	sqlite3_bind_int(stmt, 1, ts);
    sqlite3_bind_text(stmt, 2, hs, strlen(hs), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, ad, strlen(ad), SQLITE_TRANSIENT);
	struct Data* data = runSelect(stmt);

	finishedStmt(stmt);

	return data;
}
