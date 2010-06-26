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

#include <unistd.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

#define BUSY_WAIT_INTERVAL 30000

/*
Contains common database-handling routines.
*/

static sqlite3* db;
static sqlite3_stmt *stmtSelectConfig;
static int dbOpen = FALSE;
static int inTransaction = FALSE;

void prepareSql(sqlite3_stmt **stmt, const char *sql){
 // Initialise a prepared statement, using the specified SQL string - exit if there is a problem
	assert(dbOpen);
	int rc = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
	if (rc != SQLITE_OK){
		logMsg(LOG_ERR, "Unable to prepare SQL '%s' rc=%d error=%s", sql, rc, sqlite3_errmsg(db));
		exit(1);
	}
}

sqlite3* openDb(){
 // Open the database file, exit if there is a problem
	assert(!dbOpen);

 // Get the db location
	char dbPath[MAX_PATH_LEN];
	getDbPath(dbPath);

 // If we are using a real db file (not an in-memory d/b) then check that it exists
    int inMemDb = strcmp(IN_MEMORY_DB, dbPath);
	if ((inMemDb != 0) && (access(dbPath , F_OK) == -1 )) {
		logMsg(LOG_ERR, "The database file %s does not exist", dbPath);
		exit(1);
	}

 // Open the database
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
		logMsg(LOG_ERR, "Unable to open database %s rc=%d error=%s", dbPath, rc, sqlite3_errmsg(db));
		exit(1);
	}

	dbOpen = TRUE;
	setBusyWait(BUSY_WAIT_INTERVAL);

	return db;
}

int isDbOpen(){
    return dbOpen;
}

void setBusyWait(int waitInMs){
    sqlite3_busy_timeout(db, waitInMs);
}

void dbVersionCheck(){
 // For some apps we don't want to proceed if the d/b version is not what we expect
    assert(dbOpen);

    int dbVersion = getDbVersion();
    if (dbVersion != DB_VERSION){
        closeDb();

        char dbPath[MAX_PATH_LEN];
        getDbPath(dbPath);
		logMsg(LOG_ERR, "Bad database version detected. This application requires a database at version %d but the file %s has version %d",
            DB_VERSION, dbPath, dbVersion);

		exit(1);
    }
}

void prepareDb(){
	assert(dbOpen);
	prepareSql(&stmtSelectConfig, "SELECT value FROM config WHERE key=?");
}

int executeSql(const char* sql, int (*callback)(void*, int, char**, char**) ){
    char* errMsg;
    int rc = sqlite3_exec(db, sql, callback, NULL, &errMsg);
	if (rc == SQLITE_OK){
	    return SUCCESS;
	} else {
		printf("Unable to execute sql '%s'. rc=%d msg=%s", sql, rc, errMsg);
		sqlite3_free(errMsg);
		return FAIL;
	}
}

static struct Data* dataForRow(int colCount, sqlite3_stmt *stmt){
 // Helper function that creates a new Data struct for the current row returned by the stmt argument
 // NOTE need to use 'AS' for each column in the SQL for this to work
	assert(dbOpen);
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
            const unsigned char* addr = sqlite3_column_text(stmt, col);
            setAddress(data, addr);

		} else if (strcmp(colName, "hs") == 0){
            const unsigned char* host = sqlite3_column_text(stmt, col);
            setHost(data, host);

		} else {
			// ignore
		}
	}

	return data;
}

void runSelectAndCallback(sqlite3_stmt *stmt, void (*callback)(struct Data*, int), int handle){
 // Runs the stmt (assumed to be a SQL SELECT) and performs a callback once for each row
	assert(dbOpen);
	int colCount = sqlite3_column_count(stmt);
	int rc;

	struct Data* thisRow = NULL;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
		thisRow = dataForRow(colCount, stmt);

	 // Callback must free data
		callback(thisRow, handle);
	}
	sqlite3_reset(stmt);

	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, "runSelectAndCallback caused sqlite3_step to return %d.", rc);
	}

}

struct Data* runSelect(sqlite3_stmt *stmt){
 // Runs the stmt (assumed to be a SQL SELECT) and performs a callback once for each row
	assert(dbOpen);

	struct Data* result = NULL;
	struct Data* thisRow = NULL;

	int colCount = sqlite3_column_count(stmt);
	int rc;

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
		thisRow = dataForRow(colCount, stmt);
		appendData(&result, thisRow);
	}
	sqlite3_reset(stmt);

	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, "runSelect caused sqlite3_step to return %d.", rc);
	}

	return result;
}

void beginTrans(int immediate){
 // Start a db transaction
	assert(dbOpen);
	assert(!inTransaction); // We dont use nested tranactions

    char *errMsg;
    char *sql = (immediate == TRUE) ? "begin immediate" : "begin";
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
	if (rc != SQLITE_OK){
		logMsg(LOG_ERR, "Unable to begin new transaction. rc=%d msg=%s", rc, errMsg);
		sqlite3_free(errMsg);
	}
	inTransaction = TRUE;
}

void commitTrans(){
 // Commit the current transaction
	assert(dbOpen);
	assert(inTransaction);

    char *errMsg;
    int rc = sqlite3_exec(db, "commit", NULL, NULL, &errMsg);
    if (rc != SQLITE_OK){
        logMsg(LOG_ERR, "Unable to commit transaction. rc=%d msg=%s", rc, errMsg);
        sqlite3_free(errMsg);
    }

    inTransaction = FALSE;
}

void rollbackTrans(){
 // Rollback the current transaction
	assert(dbOpen);
	assert(inTransaction);

    char *errMsg;
    int rc = sqlite3_exec(db, "rollback", NULL, NULL, &errMsg);
    if (rc != SQLITE_OK){
        logMsg(LOG_ERR, "Unable to rollback transaction. rc=%d msg=%s", rc, errMsg);
        sqlite3_free(errMsg);
    }

    inTransaction = FALSE;
}

int getConfigInt(const char* key, int quiet){
   	assert(dbOpen);

 // Return the specified value from the 'config' table
	sqlite3_bind_text(stmtSelectConfig, 1, key, -1, SQLITE_TRANSIENT);

	int rc = sqlite3_step(stmtSelectConfig);

	int value;
	if (rc == SQLITE_ROW){
		value = sqlite3_column_int(stmtSelectConfig, 0);
	} else {
	    if (!quiet){
            logMsg(LOG_ERR, "Unable to retrieve config value for '%s' rc=%d error=%s", key, rc, getDbError());
	    }
		value = -1;
	}

  	sqlite3_reset(stmtSelectConfig);

  	return value;
}

char* getConfigText(const char* key, int quiet){
   	assert(dbOpen);

 // Return the specified value from the 'config' table
	sqlite3_bind_text(stmtSelectConfig, 1, key, -1, SQLITE_TRANSIENT);

	int rc = sqlite3_step(stmtSelectConfig);

	char* value;
	if (rc == SQLITE_ROW){
		value = strdup(sqlite3_column_text(stmtSelectConfig, 0));
	} else {
	    if (!quiet){
            logMsg(LOG_ERR, "Unable to retrieve config value for '%s' rc=%d error=%s", key, rc, getDbError());
	    }
		value = NULL;
	}

  	sqlite3_reset(stmtSelectConfig);

  	return value;
}

int setConfigIntValue(char* key, int value){
    char* currentValue = getConfigText(key, TRUE);
    char sql[100];
    if (currentValue == NULL){
        sprintf(sql, "INSERT INTO config (key, value) VALUES ('%s', %d)", key, value);
    } else {
        sprintf(sql, "UPDATE config SET key='%s', value=%d WHERE key='%s'", key, value, key);
    }
    return executeSql(sql, NULL);
}

int setConfigTextValue(char* key, char* value){
    char* currentValue = getConfigText(key, TRUE);
    char sql[200];
    if (currentValue == NULL){
        sprintf(sql, "INSERT INTO config (key, value) VALUES ('%s', '%s')", key, value);
    } else {
        sprintf(sql, "UPDATE config SET key='%s', value='%s' WHERE key='%s'", key, value, key);
    }
    return executeSql(sql, NULL);
}

int getDbVersion(){
    int dbVersion = getConfigInt(CONFIG_DB_VERSION, TRUE);
    if (dbVersion <= 0){
     // This is an early version of the d/b that doesn't have a 'db.version' config item - call this version 1
        dbVersion = 1;
    }

	return dbVersion;
}

const char* getDbError(){
 // Return the current d/b error message
    return sqlite3_errmsg(db);
}

void closeDb(){
 // Close the database
	assert(dbOpen);

	#ifdef _WIN32
        sqlite3_stmt *pStmt;
		while((pStmt = sqlite3_next_stmt(db, 0))!=0 ){
		    sqlite3_finalize(pStmt);
		}
	#endif

	sqlite3_close(db);
	dbOpen = FALSE;
}
