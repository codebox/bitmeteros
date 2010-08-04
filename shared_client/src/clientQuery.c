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
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "client.h"

/*
Contains a helper function for use by clients that need to perform database queries
based on timestamp ranges, with result grouping.
*/

#define CLIENT_QUERY_SQL_ALL           "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>? AND ts<=?"
#define CLIENT_QUERY_SQL_HOST         "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>? AND ts<=? AND hs=?"
#define CLIENT_QUERY_SQL_HOST_ADAPTER "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>? AND ts<=? AND hs=? AND ad=?"

static struct Data* doQueryForInterval(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo,
        char* hs, char* ad, void (*bindQueryParams)(sqlite3_stmt*, time_t, time_t, char*, char*));
static struct Data* doQuery(sqlite3_stmt *stmt, time_t minFrom, time_t maxTo, time_t (*getNext)(time_t), time_t (*addTo)(time_t),
        char* hs, char* ad, void (*bindQueryParams)(sqlite3_stmt*, time_t, time_t, char*, char*));


static time_t addToDateY(time_t ts);
static time_t addToDateM(time_t ts);
static time_t addToDateD(time_t ts);
static time_t addToDateH(time_t ts);

static void bindQueryParamsTs(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, char* hs, char* ad);
static void bindQueryParamsTsHs(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, char* hs, char* ad);
static void bindQueryParamsTsHsAd(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, char* hs, char* ad);

struct Data* getQueryValues(time_t tsFrom, time_t tsTo, int group, char* hs, char* ad){
    int selectByAdapter = (ad == NULL ? FALSE : TRUE);
    int selectByHost    = (hs == NULL ? FALSE : TRUE);

    sqlite3_stmt *stmt = NULL;
    void (*bindQueryParams)(sqlite3_stmt*, time_t, time_t, char*, char*);

    if (selectByAdapter == TRUE) {
        if (selectByHost == TRUE) {
            stmt = getStmt(CLIENT_QUERY_SQL_HOST_ADAPTER);
            bindQueryParams = &bindQueryParamsTsHsAd;

        } else {
         // This doesn't really make sense
            logMsg(LOG_WARN, "getQueryValues called with null 'hs' but a non-null 'ad' of %s", ad);
        }

    } else {
        if (selectByHost == TRUE) {
			stmt = getStmt(CLIENT_QUERY_SQL_HOST);
            bindQueryParams = &bindQueryParamsTsHs;

        } else {
            stmt = getStmt(CLIENT_QUERY_SQL_ALL);
            bindQueryParams = &bindQueryParamsTs;
        }
    }

    struct Data* result = NULL;
    struct ValueBounds* tsBounds = calcTsBounds(hs, ad);

    if (tsBounds != NULL){
        time_t minInDb = (time_t) tsBounds->min;
        time_t maxInDb = (time_t) tsBounds->max;
        free(tsBounds);

     /* If we are using the min d/b value then must subtract 1 hour from minInDb to ensure that the lowest
        ts row gets included in the query results */
        time_t minFrom = (minInDb > tsFrom ? minInDb - 3600 : tsFrom);
        time_t maxTo   = (maxInDb < tsTo   ? maxInDb : tsTo);

	 // Decide how the results should be grouped
        switch(group){
            case QUERY_GROUP_HOURS:
                result = doQuery(stmt, minFrom, maxTo, &getNextHourForTs, &addToDateH, hs, ad, bindQueryParams);
                break;

            case QUERY_GROUP_DAYS:
                result = doQuery(stmt, minFrom, maxTo, &getNextDayForTs, &addToDateD, hs, ad, bindQueryParams);
                break;

            case QUERY_GROUP_MONTHS:
                result = doQuery(stmt, minFrom, maxTo, &getNextMonthForTs, &addToDateM, hs, ad, bindQueryParams);
                break;

            case QUERY_GROUP_YEARS:
                result = doQuery(stmt, minFrom, maxTo, &getNextYearForTs, &addToDateY, hs, ad, bindQueryParams);
                break;

            case QUERY_GROUP_TOTAL:
                result = doQueryForInterval(stmt, minFrom, maxTo, hs, ad, bindQueryParams);
                break;

            default:
            	assert(FALSE);
        }
    }

	finishedStmt(stmt);

    return result;
}

static struct Data* doQuery(sqlite3_stmt *stmt, time_t minFrom, time_t maxTo, time_t (*getNext)(time_t), time_t (*addTo)(time_t),
        char* hs, char* ad, void (*bindQueryParams)(sqlite3_stmt*, time_t, time_t, char*, char*)){
 /* Performs a series of SELECT queries to return the amount of data recorded between minFrom
    and maxTo. The data is returned in a list of Data structs, each one covering an interval
    within the specified range. The length of the interval represented by each Data struct
    is determined by the 'addTo' function. The initial rounding-up of the beginning of the
    range is performed by the 'getNext' function. */
    time_t from, to;

	from = minFrom;
	to   = getNext(minFrom); // Work out the end of the first range that we will return

	to = (to > maxTo) ? maxTo : to;

    struct Data* result = NULL;
    struct Data* current;

	while(TRUE){
	    current = doQueryForInterval(stmt, from, to, hs, ad, bindQueryParams);
	    if (current->dl > 0 || current->ul > 0){
	     // Only return the struct if it contains some data
            appendData(&result, current);
	    } else {
	     // No data was found for this interval
	    	freeData(current);
	    }

        if (to >= maxTo){
            break;
        } else {
         // Work out the next interval that we will query
            from = to;
            to   = addTo(to);
            //to = (to > maxTo) ? maxTo : to;
        }
	}

	return result;
}

static time_t addToDateY(time_t ts){
 // Add 1 year to the specified timestamp
    return addToDate(ts, 'y', 1);
}

static time_t addToDateM(time_t ts){
 // Add 1 month to the specified timestamp
    return addToDate(ts, 'm', 1);
}

static time_t addToDateD(time_t ts){
 // Add 1 day to the specified timestamp
    return addToDate(ts, 'd', 1);
}

static time_t addToDateH(time_t ts){
 // Add 1 hour to the specified timestamp
    return addToDate(ts, 'h', 1);
}

static struct Data* doQueryForInterval(sqlite3_stmt *stmt, time_t tsFrom, time_t tsTo, char* hs, char* ad,
        void (*bindQueryParams)(sqlite3_stmt*, time_t, time_t, char*, char*)){
 // Perform a SQL SELECT for the specified interval
	bindQueryParams(stmt, tsFrom, tsTo, hs, ad);

	struct Data* data = runSelect(stmt);
	sqlite3_reset(stmt);

	data->ts = tsTo;
	data->dr = tsTo - tsFrom;

	assert(data->next == NULL); // We should only ever have 1 row

	return data;
}

static void bindQueryParamsTs(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, char* hs, char* ad) {
	sqlite3_bind_int(stmt, 1, tsFrom);
	sqlite3_bind_int(stmt, 2, tsTo);
 // hs and ad are ignored
}

static void bindQueryParamsTsHs(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, char* hs, char* ad) {
	sqlite3_bind_int(stmt, 1, tsFrom);
	sqlite3_bind_int(stmt, 2, tsTo);
    sqlite3_bind_text(stmt, 3, hs, strlen(hs), SQLITE_TRANSIENT);
 // ad is ignored
}

static void bindQueryParamsTsHsAd(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, char* hs, char* ad) {
	sqlite3_bind_int(stmt, 1, tsFrom);
	sqlite3_bind_int(stmt, 2, tsTo);
    sqlite3_bind_text(stmt, 3, hs, strlen(hs), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, ad, strlen(ad), SQLITE_TRANSIENT);
}
