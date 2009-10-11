#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include "common.h"
#include "client.h"

static sqlite3_stmt *stmt = NULL;
static pthread_mutex_t stmtMutex = PTHREAD_MUTEX_INITIALIZER;

void getDumpValues(void (*callback)(struct Data*)){
    pthread_mutex_lock(&stmtMutex);

    if (stmt == NULL){
         prepareSql(&stmt, "SELECT ts AS ts, dr AS dr, dl AS dl, ul AS ul, ad AS ad FROM data ORDER BY ts DESC");
    }

    runSelectAndCallback(stmt, callback);
    pthread_mutex_unlock(&stmtMutex);
}

