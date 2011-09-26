#ifdef UNIT_TESTING 
	#include "test.h"
#endif
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

#define CLIENT_QUERY_SQL "SELECT SUM(vl) AS vl FROM data2 WHERE fl=? AND ts>? AND ts<=?"

static struct Data* doQueryForInterval(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, int fl);
static struct Data* doQuery(sqlite3_stmt *stmt, time_t minFrom, time_t maxTo, time_t (*getNext)(time_t), time_t (*addTo)(time_t), int fl);

static time_t addToDateY(time_t ts);
static time_t addToDateM(time_t ts);
static time_t addToDateD(time_t ts);
static time_t addToDateH(time_t ts);

static void bindQueryParams(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, int fl);

struct Data* getQueryValues(time_t tsFrom, time_t tsTo, int group, int fl){
    sqlite3_stmt *stmt = getStmt(CLIENT_QUERY_SQL);

    struct Data* result = NULL;
    struct ValueBounds* tsBounds = calcTsBounds(fl);

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
                result = doQuery(stmt, minFrom, maxTo, &getNextHourForTs, &addToDateH, fl);
                break;

            case QUERY_GROUP_DAYS:
                result = doQuery(stmt, minFrom, maxTo, &getNextDayForTs, &addToDateD, fl);
                break;

            case QUERY_GROUP_MONTHS:
                result = doQuery(stmt, minFrom, maxTo, &getNextMonthForTs, &addToDateM, fl);
                break;

            case QUERY_GROUP_YEARS:
                result = doQuery(stmt, minFrom, maxTo, &getNextYearForTs, &addToDateY, fl);
                break;

            case QUERY_GROUP_TOTAL:
                result = doQueryForInterval(stmt, minFrom, maxTo, fl);
                break;

            default:
            	assert(FALSE);
        }
    }

	finishedStmt(stmt);

    return result;
}

static struct Data* doQuery(sqlite3_stmt *stmt, time_t minFrom, time_t maxTo, time_t (*getNext)(time_t), time_t (*addTo)(time_t), int fl){
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
	    current = doQueryForInterval(stmt, from, to, fl);
	    if (current->vl > 0){
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

static struct Data* doQueryForInterval(sqlite3_stmt *stmt, time_t tsFrom, time_t tsTo, int fl){
 // Perform a SQL SELECT for the specified interval
	bindQueryParams(stmt, tsFrom, tsTo, fl);

	struct Data* data = runSelect(stmt);
	sqlite3_reset(stmt);

	data->ts = tsTo;
	data->dr = tsTo - tsFrom;
	data->fl = fl;

	assert(data->next == NULL); // We should only ever have 1 row

	return data;
}

static void bindQueryParams(sqlite3_stmt* stmt, time_t tsFrom, time_t tsTo, int fl) {
	sqlite3_bind_int(stmt, 1, fl);
	sqlite3_bind_int(stmt, 2, tsFrom);
	sqlite3_bind_int(stmt, 3, tsTo);
}

