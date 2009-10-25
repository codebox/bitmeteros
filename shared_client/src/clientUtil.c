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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "common.h"
#include "client.h"

/*
Contains thread-safe utility functions used by other client modules.
*/

static pthread_mutex_t stmtTsBoundsMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t stmtMaxValuesMutex = PTHREAD_MUTEX_INITIALIZER;
static sqlite3_stmt *stmtTsBounds  = NULL;
static sqlite3_stmt *stmtMaxValues = NULL;

struct ValueBounds* calcTsBounds(){
 /* Calculate the smallest and largest timestamps in the data table. If the table is
    empty then we return NULL. */

	pthread_mutex_lock(&stmtTsBoundsMutex);
	if (stmtTsBounds == NULL){
        prepareSql(&stmtTsBounds, "SELECT MIN(ts), MAX(ts) FROM data");
	}

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

	sqlite3_reset(stmtTsBounds);
    pthread_mutex_unlock(&stmtTsBoundsMutex);

	return values;
}

struct Data* calcMaxValues(){
 // Calculate the largest ul and dl values that exist in the data table
	pthread_mutex_lock(&stmtMaxValuesMutex);
	if (stmtMaxValues == NULL){
        prepareSql(&stmtMaxValues, "SELECT MAX(dl) AS dl, MAX(ul) AS ul FROM data");
	}

    struct Data* result = runSelect(stmtMaxValues);
    pthread_mutex_unlock(&stmtMaxValuesMutex);

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
			formatAmount(dl, 0, (units == UNITS_ABBREV), dlTxt);
			formatAmount(ul, 0, (units == UNITS_ABBREV), ulTxt);
			break;

		default:
			assert(FALSE); // We validate for bad unit values in the options module
			break;
	}
}
