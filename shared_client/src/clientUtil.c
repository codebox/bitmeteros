#include <sqlite3.h>
#include <pthread.h>
#include "common.h"
#include "client.h"

static pthread_mutex_t stmtTsBoundsMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t stmtMaxValuesMutex = PTHREAD_MUTEX_INITIALIZER;
static sqlite3_stmt *stmtTsBounds  = NULL;
static sqlite3_stmt *stmtMaxValues = NULL;

struct ValueBounds* calcTsBounds(){
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
	pthread_mutex_lock(&stmtMaxValuesMutex);
	if (stmtMaxValues == NULL){
        prepareSql(&stmtMaxValues, "SELECT MAX(dl) AS dl, MAX(ul) AS ul FROM data");
	}

    struct Data* result = runSelect(stmtMaxValues);
    pthread_mutex_unlock(&stmtMaxValuesMutex);

	return result;
}

