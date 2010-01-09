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

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "common.h"
#include "client.h"

/*
Contains thread-safe utility functions used by other client modules.
*/

#ifndef MULTI_THREADED_CLIENT
    static sqlite3_stmt *stmtTsBounds  = NULL;
    static sqlite3_stmt *stmtMaxValues = NULL;
#endif

#define TS_BOUNDS_SQL  "SELECT MIN(ts), MAX(ts) FROM data"
#define MAX_VALUES_SQL "SELECT MAX(dl) AS dl, MAX(ul) AS ul FROM data"

struct ValueBounds* calcTsBounds(){
 /* Calculate the smallest and largest timestamps in the data table. If the table is
    empty then we return NULL. */

	#ifdef MULTI_THREADED_CLIENT
    	sqlite3_stmt *stmtTsBounds = NULL;
    	prepareSql(&stmtTsBounds, TS_BOUNDS_SQL);
    #else
        if (stmtTsBounds == NULL){
            prepareSql(&stmtTsBounds, TS_BOUNDS_SQL);
        }
    #endif

	time_t minTs, maxTs;
	struct ValueBounds* values = NULL;
	int rc = sqlite3_step(stmtTsBounds);
	if (rc == SQLITE_ROW){
		minTs = sqlite3_column_int(stmtTsBounds, 0);
		maxTs = sqlite3_column_int(stmtTsBounds, 1);

        if (maxTs > 0){
         // Need a way to show that the table contains no data - if maxTs==0 then no rows were found so we will return NULL
            values = malloc(sizeof(struct ValueBounds));
            values->min = minTs;
            values->max = maxTs;
        }
	}

	#ifdef MULTI_THREADED_CLIENT
    	sqlite3_finalize(stmtTsBounds);
    #else
    	sqlite3_reset(stmtTsBounds);
    #endif

	return values;
}

struct Data* calcMaxValues(){
 // Calculate the largest ul and dl values that exist in the data table
    #ifdef MULTI_THREADED_CLIENT
    	sqlite3_stmt *stmtMaxValues = NULL;
    	prepareSql(&stmtMaxValues, MAX_VALUES_SQL);
    #else
    	if (stmtMaxValues == NULL){
    		prepareSql(&stmtMaxValues, MAX_VALUES_SQL);
    	}
    #endif

	struct Data* result = runSelect(stmtMaxValues);

	#ifdef MULTI_THREADED_CLIENT
    	sqlite3_finalize(stmtMaxValues);
    #else
    	sqlite3_reset(stmtMaxValues);
    #endif

    return result;
}

void formatAmounts(const BW_INT dl, const BW_INT ul, char* dlTxt, char *ulTxt, int units){
	switch (units) {
		case UNITS_BYTES:
			sprintf(dlTxt, "%llu", dl);
			sprintf(ulTxt, "%llu", ul);
			break;

		case UNITS_ABBREV:
		case UNITS_FULL:
			formatAmount(dl, TRUE, (units == UNITS_ABBREV), dlTxt);
			formatAmount(ul, TRUE, (units == UNITS_ABBREV), ulTxt);
			break;

		default:
			assert(FALSE); // We validate for bad unit values in the options module
			break;
	}
}
