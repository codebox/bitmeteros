/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include "client.h"

/*
Contains thread-safe utility functions used by other client modules.
*/

#define TS_BOUNDS_SQL_ALL          "SELECT MIN(ts), MAX(ts) FROM data"
#define TS_BOUNDS_SQL_HOST         "SELECT MIN(ts), MAX(ts) FROM data WHERE hs = ?"
#define TS_BOUNDS_SQL_HOST_ADAPTER "SELECT MIN(ts), MAX(ts) FROM data WHERE hs = ? AND ad = ?"

#define MAX_VALUES_SQL "SELECT MAX(dl) AS dl, MAX(ul) AS ul FROM data"

static struct ValueBounds* calcTsBoundsAll(char* hs, char* ad);
static struct ValueBounds* calcTsBoundsHost(char* hs, char* ad);
static struct ValueBounds* calcTsBoundsHostAdapter(char* hs, char* ad);

struct ValueBounds* calcTsBounds(char* hs, char* ad){
 /* Calculate the smallest and largest timestamps in the data table that match the
    specified host/adapter combination. If are found then we return NULL. */
    int selectByAdapter = (ad == NULL ? FALSE : TRUE);
    int selectByHost    = (hs == NULL ? FALSE : TRUE);

    struct ValueBounds* (*calc)(char*, char*);

    if (selectByAdapter == TRUE) {
        if (selectByHost == TRUE) {
         // We want data only for 1 specific adapter on 1 host
            calc = &calcTsBoundsHostAdapter;

        } else {
         // This doesn't really make sense
            logMsg(LOG_WARN, "calcTsBounds called with null 'hs' but a non-null 'ad' of %s", ad);
            calc = NULL;
        }

    } else {
        if (selectByHost == TRUE) {
         // We want data only for 1 specific host
            calc = &calcTsBoundsHost;

        } else {
         // We want all data regardless of where it is from
            calc = &calcTsBoundsAll;
        }
    }

	return calc(hs, ad);
}

static struct ValueBounds* buildTsBounds(sqlite3_stmt* stmtTsBounds){
    time_t minTs, maxTs;
	struct ValueBounds* values = NULL;
	int rc = sqlite3_step(stmtTsBounds);

	if (rc == SQLITE_ROW){
		minTs = sqlite3_column_int(stmtTsBounds, 0);
		maxTs = sqlite3_column_int(stmtTsBounds, 1);

        if (maxTs > 0){
         // Need a way to show that the table contains no matches - if maxTs==0 then no rows were found so we will return NULL
            values = malloc(sizeof(struct ValueBounds));
            values->min = minTs;
            values->max = maxTs;
        }
	}

    return values;
}

static struct ValueBounds* calcTsBoundsAll(char* hs, char* ad){
   	sqlite3_stmt *stmtTsBounds = getStmt(TS_BOUNDS_SQL_ALL);

    struct ValueBounds* tsBounds = buildTsBounds(stmtTsBounds);

	finishedStmt(stmtTsBounds);

    return tsBounds;
}

static struct ValueBounds* calcTsBoundsHost(char* hs, char* ad){
    sqlite3_stmt *stmtTsBounds = getStmt(TS_BOUNDS_SQL_HOST);

    sqlite3_bind_text(stmtTsBounds, 1, hs, strlen(hs), SQLITE_TRANSIENT);
    struct ValueBounds* tsBounds = buildTsBounds(stmtTsBounds);

	finishedStmt(stmtTsBounds);

    return tsBounds;
}
static struct ValueBounds* calcTsBoundsHostAdapter(char* hs, char* ad){
    sqlite3_stmt *stmtTsBounds = getStmt(TS_BOUNDS_SQL_HOST_ADAPTER);

    sqlite3_bind_text(stmtTsBounds, 1, hs, strlen(hs), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmtTsBounds, 2, ad, strlen(ad), SQLITE_TRANSIENT);

    struct ValueBounds* tsBounds = buildTsBounds(stmtTsBounds);

	finishedStmt(stmtTsBounds);

    return tsBounds;
}

struct Data* calcMaxValues(){
 // Calculate the largest ul and dl values that exist in the data table
   	sqlite3_stmt *stmtMaxValues= getStmt(MAX_VALUES_SQL);

	struct Data* result = runSelect(stmtMaxValues);

	finishedStmt(stmtMaxValues);

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

struct HostAdapter* getHostAdapter(char* hostAndAdapterTxt){
    struct HostAdapter* hostAdapter = NULL;

    if (hostAndAdapterTxt != NULL) {
        hostAdapter = malloc(sizeof(struct HostAdapter));

        char* host    = strdup(hostAndAdapterTxt);
        char* adapter = NULL;

     /* The 'hostAndAdapterTxt' variable can contain either just a host name, or a host name/adapter
        combination, with the 2 values separated with a colon. */
        char* colonPtr = strchr(hostAndAdapterTxt, ':');

        if (colonPtr != NULL){
         // Split host:adapter into the 2 parts
            int hostLen = colonPtr - hostAndAdapterTxt;
            strncpy(host, hostAndAdapterTxt, hostLen);
            host[hostLen] = 0;

            adapter = colonPtr + 1;
        }

     /* A value of 'local' is used to indicate the special case where data from the local host
        should be included. This avoids the counter-intuitive command-line syntax that would otherwise
        be required to obtain this information (ie supplying the -a option with no value following it). */
        if (strcmp(LOCAL_HOST, host) == 0){
            hostAdapter->host = strdup("");
        } else {
            hostAdapter->host = strdup(host);
        }

        if (adapter != NULL){
            hostAdapter->adapter = strdup(adapter);
        } else {
            hostAdapter->adapter = NULL;
        }

        free(host);
	}

	return hostAdapter;
}

void freeHostAdapter(struct HostAdapter *hostAdapter){
    if (hostAdapter != NULL){
        if (hostAdapter->host != NULL){
            free(hostAdapter->host);
        }
        if (hostAdapter->adapter != NULL){
            free(hostAdapter->adapter);
        }
        free(hostAdapter);
    }
}
