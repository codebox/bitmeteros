#include <sqlite3.h>
#include <pthread.h>
#include "common.h"
#include "client.h"

static sqlite3_stmt *stmt = NULL;
static pthread_mutex_t stmtMutex = PTHREAD_MUTEX_INITIALIZER;
struct Summary getSummaryValues();
static struct Data* calcTotalsSince(int ts);

struct Summary getSummaryValues(){
    pthread_mutex_lock(&stmtMutex);

    if (stmt == NULL){
        prepareSql(&stmt, "SELECT sum(dl) AS dl, sum(ul) AS ul FROM data WHERE ts>=?");
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
        summary.tsMin = 0;
        summary.tsMax = 0; //TODO check for this in bmclient
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

