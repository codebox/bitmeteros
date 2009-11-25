/*
 * BitMeterOS v0.2.0
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
 * Build Date: Wed, 25 Nov 2009 10:48:23 +0000
 */

#include <sqlite3.h>
#include <stdlib.h>
#include "common.h"
#include "client.h"

/*
Contains a helper function for use by clients that need to produce database summaries.
*/

#define CLIENT_SUMMARY_SQL "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>=?"

#ifndef MULTI_THREADED_CLIENT
	static sqlite3_stmt *stmt = NULL;
#endif

struct Summary getSummaryValues();
static struct Data* calcTotalsSince(sqlite3_stmt *stmt, int ts);

struct Summary getSummaryValues(){
 // Populate a Summary struct
 
 	#ifdef MULTI_THREADED_CLIENT
    	sqlite3_stmt *stmt = NULL;
    	prepareSql(&stmt, CLIENT_SUMMARY_SQL);
    #else
    	if (stmt == NULL){
    		prepareSql(&stmt, CLIENT_SUMMARY_SQL);
    	}
    #endif	

	struct Summary summary;
	int now = getTime();

	int tsForStartOfToday = getCurrentDayForTs(now);
	summary.today = calcTotalsSince(stmt, tsForStartOfToday);

	int tsForStartOfMonth = getCurrentMonthForTs(now);
	summary.month = calcTotalsSince(stmt, tsForStartOfMonth);

	int tsForStartOfYear = getCurrentYearForTs(now);
	summary.year = calcTotalsSince(stmt, tsForStartOfYear);

	summary.total = calcTotalsSince(stmt, 0);

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

	#ifdef MULTI_THREADED_CLIENT
    	sqlite3_finalize(stmt);
    #else
    	sqlite3_reset(stmt);
    #endif	
    
	return summary;
}

static struct Data* calcTotalsSince(sqlite3_stmt *stmt, int ts){
	sqlite3_bind_int(stmt, 1, ts);
	struct Data* data = runSelect(stmt);
	sqlite3_reset(stmt);

	return data;
}

