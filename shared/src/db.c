#include <unistd.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

static sqlite3* db;

void prepareSql(sqlite3_stmt **stmt, const char *sql){
	//TODO assert open
	int rc = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
	if (rc != SQLITE_OK){
		logMsg(LOG_ERR, "Unable to prepare SQL '%s' rc=%d error=%s\n", sql, rc, sqlite3_errmsg(db));
		exit(1);
	}
}

sqlite3* openDb(){
	char dbPath[MAX_PATH_LEN];
	getDbPath(dbPath);

    int inMemDb = strcmp(IN_MEMORY_DB, dbPath);
	if ((inMemDb != 0) && (access(dbPath , F_OK) == -1 )) {
		logMsg(LOG_ERR, "The database file %s does not exist\n", dbPath);
		exit(1);
	}

	//TODO assert not already open
	#ifdef __linux__
		int rc = sqlite3_open(dbPath, &db);
	#endif
	#ifdef _WIN32
		int rc = sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READWRITE, NULL);
	#endif
	#ifdef __APPLE__
		int rc = sqlite3_open(dbPath, &db);
	#endif
	if (rc != SQLITE_OK){
		logMsg(LOG_ERR, "Unable to open database %s rc=%d error=%s\n", dbPath, rc, sqlite3_errmsg(db));
		exit(1);
	}
	return db;
}

void executeSql(const char* sql, int (*callback)(void*, int, char**, char**) ){ //TODO still need this?
    char* errMsg;
    int rc = sqlite3_exec(db, sql, callback, NULL, &errMsg);
	if (rc != SQLITE_OK){
		printf("Unable to execute sql '%s'. rc=%d msg=%s\n", sql, rc, errMsg);
		sqlite3_free(errMsg);
		return;
	}
}
struct Data* dataForRow(int colCount, sqlite3_stmt *stmt){
	int col;
	const char* colName;
	struct Data* data = allocData();

	for (col=0; col<colCount; col++) {
		colName = sqlite3_column_name(stmt, col);

		if (strcmp(colName, "ts") == 0){
			data->ts = sqlite3_column_int(stmt, col);
		} else if (strcmp(colName, "dl") == 0){
			data->dl = sqlite3_column_int64(stmt, col);
		} else if (strcmp(colName, "ul") == 0){
			data->ul = sqlite3_column_int64(stmt, col);
		} else if (strcmp(colName, "dr") == 0){
			data->dr = sqlite3_column_int(stmt, col);
		} else if (strcmp(colName, "ad") == 0){
            char* addr = sqlite3_column_text(stmt, col);
            data->ad = malloc(strlen(addr) + 1);
            strcpy(data->ad, addr);
		} else {
			// ignore
		}
	}

	return data;
}
void runSelectAndCallback(sqlite3_stmt *stmt, void (*callback)(struct Data*)){
	int colCount = sqlite3_column_count(stmt);
	int rc;

	struct Data* thisRow = NULL;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
		thisRow = dataForRow(colCount, stmt);

	 // Callback must free data
		callback(thisRow);
	}
	sqlite3_reset(stmt);
}
struct Data* runSelect(sqlite3_stmt *stmt){
 // NOTE need to use 'AS' for each column in the SQL for this to work
	struct Data* result = NULL;
	struct Data* thisRow = NULL;
	struct Data* prevRow = NULL;

	int colCount = sqlite3_column_count(stmt);
	int rc;

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
		thisRow = dataForRow(colCount, stmt);

		if (result == NULL){
			result = thisRow;
		} else {
			prevRow->next = thisRow;
		}
		prevRow = thisRow;
	}
	sqlite3_reset(stmt);
	//TODO check rc for errors
	return result;
}

void beginTrans(){
    char *errMsg;
    int rc = sqlite3_exec(db, "begin", NULL, NULL, &errMsg);
	if (rc != SQLITE_OK){
		logMsg(LOG_ERR, "Unable to begin new transaction. rc=%d msg=%s\n", rc, errMsg);
		sqlite3_free(errMsg);
	}
}

void commitTrans(){
    char *errMsg;
    int rc = sqlite3_exec(db, "commit", NULL, NULL, &errMsg);
    if (rc != SQLITE_OK){
        logMsg(LOG_ERR, "Unable to commit transaction. rc=%d msg=%s\n", rc, errMsg);
        sqlite3_free(errMsg);
    }
}

void rollbackTrans(){
    char *errMsg;
    int rc = sqlite3_exec(db, "rollback", NULL, NULL, &errMsg);
    if (rc != SQLITE_OK){
        logMsg(LOG_ERR, "Unable to rollback transaction. rc=%d msg=%s\n", rc, errMsg);
        sqlite3_free(errMsg);
    }
}

const char* getDbError(){
    return sqlite3_errmsg(db);
}

void setDbBusyWait(int interval){
    sqlite3_busy_timeout(db, interval);
}

void closeDb(){
	//TODO assert open
	#ifdef __linux__
		//TODO
	#endif
	#ifdef __APPLE__
		//TODO
	#endif
	#ifdef _WIN32
        sqlite3_stmt *pStmt;
		while((pStmt = sqlite3_next_stmt(db, 0))!=0 ){
		    sqlite3_finalize(pStmt);
		}
	#endif

	sqlite3_close(db);
}
