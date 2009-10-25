/*
 * BitMeterOS v0.1.5
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
 * Build Date: Sun, 25 Oct 2009 17:18:38 +0000
 */

#include <sqlite3.h>
#include <pthread.h>
#include <stdlib.h>
#include "common.h"
#include "client.h"

/*
Contains a thread-safe helper function for use by clients that need to produce database summaries.
*/

static sqlite3_stmt *stmt = NULL;
static pthread_mutex_t stmtMutex = PTHREAD_MUTEX_INITIALIZER;
struct Summary getSummaryValues();
static struct Data* calcTotalsSince(int ts);

struct Summary getSummaryValues(){
 // Populate a Summary struct
    pthread_mutex_lock(&stmtMutex);

    if (stmt == NULL){
        prepareSql(&stmt, "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>=?");
    }

	struct Summary summary;
	int now = getTime();

	int tsForStartOfToday = getCurrentDayForTs(now);
	summary.today = calcTotalsSince(tsForStartOfToday);

	int tsForStartOfMonth = getCurrentMonthForTs(now);
	summary.month = calcTotalsSince(tsForStartOfMonth);

	int tsForStartOfYear = getCurrentYearForTs(now);
	summary.year = calcTotalsSince(tsForStartOfYear);

	summary.total = calcTotalsSince(0);

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

    pthread_mutex_unlock(&stmtMutex);

	return summary;
}

static struct Data* calcTotalsSince(int ts){
	sqlite3_bind_int(stmt, 1, ts);
	struct Data* data = runSelect(stmt);
	sqlite3_reset(stmt);

	return data;
}

