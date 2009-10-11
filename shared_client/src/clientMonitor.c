#include <sqlite3.h>
#include <pthread.h>
#include "common.h"

static sqlite3_stmt *stmt = NULL;
static pthread_mutex_t stmtMutex = PTHREAD_MUTEX_INITIALIZER;

struct Data* getMonitorValues(int ts){
    pthread_mutex_lock(&stmtMutex);

    if (stmt == NULL){
        prepareSql(&stmt, "SELECT ts AS ts, SUM(dl) AS dl, SUM(ul) AS ul FROM data GROUP BY ts HAVING ts >= ? ORDER BY ts DESC");
    }

	sqlite3_bind_int(stmt, 1, ts);
	struct Data* result = runSelect(stmt);
	sqlite3_reset(stmt);

    pthread_mutex_unlock(&stmtMutex);

	return result;
}
