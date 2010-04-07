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

#define DATA_SUMMARY_SQL "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>=?"
#define HOST_SUMMARY_SQL "SELECT DISTINCT(hs) AS hs FROM data WHERE hs NOT NULL"

#ifndef MULTI_THREADED_CLIENT
	static sqlite3_stmt *stmtData = NULL;
	static sqlite3_stmt *stmtHost = NULL;
#endif

struct Summary getSummaryValues();
static struct Data* calcTotalsSince(sqlite3_stmt *stmt, int ts);
static void getHosts(sqlite3_stmt *stmt, char*** hostNames, int* hostCount);

struct Summary getSummaryValues(){
 // Populate a Summary struct

 	#ifdef MULTI_THREADED_CLIENT
    	sqlite3_stmt *stmtData = NULL;
    	prepareSql(&stmtData, DATA_SUMMARY_SQL);

    	sqlite3_stmt *stmtHost = NULL;
    	prepareSql(&stmtHost, HOST_SUMMARY_SQL);

    #else
    	if (stmtData == NULL){
    		prepareSql(&stmtData, DATA_SUMMARY_SQL);
    	}

    	if (stmtHost == NULL){
    		prepareSql(&stmtHost, HOST_SUMMARY_SQL);
    	}
    #endif

	struct Summary summary;
	int now = getTime();

	int tsForStartOfToday = getCurrentDayForTs(now);
	summary.today = calcTotalsSince(stmtData, tsForStartOfToday);

	int tsForStartOfMonth = getCurrentMonthForTs(now);
	summary.month = calcTotalsSince(stmtData, tsForStartOfMonth);

	int tsForStartOfYear = getCurrentYearForTs(now);
	summary.year = calcTotalsSince(stmtData, tsForStartOfYear);

	summary.total = calcTotalsSince(stmtData, 0);

	struct ValueBounds* tsBounds = calcTsBounds();
	if (tsBounds != NULL){
        summary.tsMin = tsBounds->min;
        summary.tsMax = tsBounds->max;
        free(tsBounds);
	} else {
	 // The data table is empty
        summary.tsMin = 0;
        summary.tsMax = 0;
	}

    getHosts(stmtHost, &(summary.hostNames), &(summary.hostCount));

	#ifdef MULTI_THREADED_CLIENT
    	sqlite3_finalize(stmtData);
    	sqlite3_finalize(stmtHost);
    #else
    	sqlite3_reset(stmtData);
    	sqlite3_reset(stmtHost);
    #endif

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


static void getHosts(sqlite3_stmt *stmt, char*** hostNames, int* hostCount){
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
}

static struct Data* calcTotalsSince(sqlite3_stmt *stmt, int ts){
	sqlite3_bind_int(stmt, 1, ts);
	struct Data* data = runSelect(stmt);
	sqlite3_reset(stmt);

	return data;
}

