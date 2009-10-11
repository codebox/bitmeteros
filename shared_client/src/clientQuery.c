#include <sqlite3.h>
#include <pthread.h>
#include <time.h>
#include "common.h"
#include "client.h"

static sqlite3_stmt *stmt = NULL;
static pthread_mutex_t stmtMutex = PTHREAD_MUTEX_INITIALIZER;
static struct Data* doQueryForInterval(time_t tsFrom, time_t tsTo);
static struct Data* doQuery(time_t minFrom, time_t maxTo, int (*getNext)(int), time_t (*addTo)(time_t));
static time_t addToDateY(time_t ts);
static time_t addToDateM(time_t ts);
static time_t addToDateD(time_t ts);
static time_t addToDateH(time_t ts);

struct Data* getQueryValues(time_t tsFrom, time_t tsTo, int group){
    pthread_mutex_lock(&stmtMutex);

    if (stmt == NULL){
         prepareSql(&stmt, "SELECT SUM(dl) AS dl, SUM(ul) AS ul FROM data WHERE ts>? AND ts<=?");
    }

    struct Data* result = NULL;
    struct ValueBounds* tsBounds = calcTsBounds();

    if (tsBounds != NULL){
        time_t minInDb = (time_t) tsBounds->min;
        time_t maxInDb = (time_t) tsBounds->max;
        free(tsBounds);

     /* If we are using the min d/b value then must subtract 1 from minInDb to ensure that the lowest
        ts row gets included in the query results */
        time_t minFrom = (minInDb > tsFrom ? minInDb-1 : tsFrom);
        time_t maxTo   = (maxInDb < tsTo   ? maxInDb   : tsTo);

        switch(group){
            case QUERY_GROUP_HOURS:
                result = doQuery(minFrom, maxTo, &getNextHourForTs, &addToDateH);
                break;
            case QUERY_GROUP_DAYS:
                result = doQuery(minFrom, maxTo, &getNextDayForTs, &addToDateD);
                break;
            case QUERY_GROUP_MONTHS:
                result = doQuery(minFrom, maxTo, &getNextMonthForTs, &addToDateM);
                break;
            case QUERY_GROUP_YEARS:
                result = doQuery(minFrom, maxTo, &getNextYearForTs, &addToDateY);
                break;
            case QUERY_GROUP_TOTAL:
                result = doQueryForInterval(minFrom, maxTo);
                break;
        }
    }

    pthread_mutex_unlock(&stmtMutex);

    return result;
}

static struct Data* doQuery(time_t minFrom, time_t maxTo, int (*getNext)(int), time_t (*addTo)(time_t)){
    time_t from, to;

	from = minFrom;
	to   = getNext(minFrom);
    struct Data* result = NULL;
    struct Data* current;

	while(to <= maxTo){
	    current = doQueryForInterval(from, to);
	    if (current->dl > 0 || current->ul > 0){
            appendData(&result, current);
	    }
		from = to;
		to   = addTo(to);
	}
	//appendData(&result, doQueryForInterval(from, maxTo));

	return result;
}

static time_t addToDateY(time_t ts){
    return addToDate(ts, 'y', 1);
}


static time_t addToDateM(time_t ts){
    return addToDate(ts, 'm', 1);
}

static time_t addToDateD(time_t ts){
    return addToDate(ts, 'd', 1);
}

static time_t addToDateH(time_t ts){
    return addToDate(ts, 'h', 1);
}


static struct Data* doQueryForInterval(time_t tsFrom, time_t tsTo){
	sqlite3_bind_int(stmt, 1, tsFrom);
	sqlite3_bind_int(stmt, 2, tsTo);

	struct Data* data = runSelect(stmt);
	sqlite3_reset(stmt);

	data->ts = tsTo;
	data->dr = tsTo - tsFrom;

	return data;
}

