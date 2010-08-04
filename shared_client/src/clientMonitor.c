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
#include <string.h>
#include "common.h"

/*
Contains a helper function for use by clients that need to monitor the database.
*/

#define CLIENT_MONITOR_SQL_ALL          "SELECT ts AS ts, dr AS dr, SUM(dl) AS dl, SUM(ul) AS ul FROM data GROUP BY ts HAVING ts >= ? ORDER BY ts DESC"
#define CLIENT_MONITOR_SQL_HOST         "SELECT ts AS ts, dr AS dr, SUM(dl) AS dl, SUM(ul) AS ul FROM data GROUP BY ts,hs HAVING ts >= ? AND hs = ? ORDER BY ts DESC"
#define CLIENT_MONITOR_SQL_HOST_ADAPTER "SELECT ts AS ts, dr AS dr, SUM(dl) AS dl, SUM(ul) AS ul FROM data GROUP BY ts,hs,ad HAVING ts >= ? AND hs = ? AND ad = ? ORDER BY ts DESC"

static struct Data* getMonitorValuesForAll(int ts);
static struct Data* getMonitorValuesForHost(int ts, char* hs);
static struct Data* getMonitorValuesForHostAndAdapter(int ts, char* hs, char* ad);

struct Data* getMonitorValues(int ts, char* hs, char* ad){
 // A list of Data structs will be returned, once for each db entry with a timestamp >= ts
    int selectByAdapter = (ad == NULL ? FALSE : TRUE);
    int selectByHost    = (hs == NULL ? FALSE : TRUE);

    struct Data* result;

    if (selectByAdapter == TRUE) {
        if (selectByHost == TRUE) {
         // We want data only for 1 specific adapter on 1 host
            result = getMonitorValuesForHostAndAdapter(ts, hs, ad);

        } else {
         // This doesn't really make sense
            logMsg(LOG_WARN, "getMonitorValues called with null 'hs' but a non-null 'ad' of %s", ad);
            result = NULL;
        }

    } else {
        if (selectByHost == TRUE) {
         // We want data only for 1 specific host
            result = getMonitorValuesForHost(ts, hs);

        } else {
         // We want all data regardless of where it is from
            result = getMonitorValuesForAll(ts);
        }
    }

	return result;
}

static struct Data* getMonitorValuesForAll(int ts){
    sqlite3_stmt *stmt = stmt = getStmt(CLIENT_MONITOR_SQL_ALL);

	sqlite3_bind_int(stmt, 1, ts);
	struct Data* result = runSelect(stmt);

	finishedStmt(stmt);

    return result;
}

static struct Data* getMonitorValuesForHost(int ts, char* hs){
    sqlite3_stmt *stmt = stmt = getStmt(CLIENT_MONITOR_SQL_HOST);

	sqlite3_bind_int(stmt, 1, ts);
	sqlite3_bind_text(stmt, 2, hs, strlen(hs), SQLITE_TRANSIENT);

	struct Data* result = runSelect(stmt);

	finishedStmt(stmt);

    return result;
}

static struct Data* getMonitorValuesForHostAndAdapter(int ts, char* hs, char* ad){
    sqlite3_stmt *stmt = stmt = getStmt(CLIENT_MONITOR_SQL_HOST_ADAPTER);

	sqlite3_bind_int(stmt, 1, ts);
	sqlite3_bind_text(stmt, 2, hs, strlen(hs), SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, ad, strlen(ad), SQLITE_TRANSIENT);

	struct Data* result = runSelect(stmt);

	finishedStmt(stmt);

    return result;
}
