#ifdef _WIN32
    #define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "capture.h"
#include <sqlite3.h>

#define SQL_INSERT_INTO_DATA    "INSERT INTO data (ts,dr,vl,fl) VALUES (?,?,?,?)"
#define SQL_SELECT_FOR_COMPRESS "SELECT fl, SUM(vl) FROM (SELECT * FROM data WHERE ts<=? AND dr=?) GROUP BY fl;"
#define SQL_SELECT_MIN_TS       "SELECT MIN(ts) FROM data WHERE dr=?"
#define SQL_DELETE_COMPRESSED   "DELETE FROM data WHERE ts<=? AND dr=?"
/*
Contains code that interacts with the database on behalf of the BitMeter Data Capture application. At a high level there
are two database operations that are performed:
  - Inserting new rows into the 'data' table (see updateDb)
  - Compressing the 'data' table by amalgamating multiple older rows with small 'dr' values into a single row with a
    larger 'dr' value (see compressDb). This compression is performed at regular intervals, and also when the application
    starts up.
*/

/* Populated from the 'config' table value with key 'cap.keep_sec_limit', this represents the age (in seconds) beyond which
   data table rows with a 'dr' value of 1 will be compressed. By default this is set to 3600, meaning that the application
   will retain second-by-second values for at least 1 hour. */
static int keepPerSecLimit;

/* Populated from the 'config' table value with key 'cap.keep_min_limit', this represents the age (in seconds) beyond which
   data table rows with a 'dr' value of 60 will be compressed. By default this is set to 86400, meaning that the application
   will retain minute-by-minute values for at least 1 day. */
static int keepPerMinLimit;

/* Populated from the 'config' table value with key 'cap.compress_interval', this represents the interval (in seconds) between
   compressDb() operations while the application is running. Note that compression may occur more frequently than this if the
   application is restarted, since a compression is performed when the app is initialised - this does no harm. */
static int compressInterval;

static int insertDataPartial(int dr, struct Data* data);
static int compressDbStage(int secKeepInterval, int oldDr, int newDr, int (*fnRoundUp)(int) );

void setupDb(){
    logMsg(LOG_DEBUG, "Starting db setup");
    
 // Read various values out of the 'config' table
    keepPerSecLimit  = getConfigInt(CONFIG_CAP_KEEP_SEC_LIMIT, FALSE);
    keepPerMinLimit  = getConfigInt(CONFIG_CAP_KEEP_MIN_LIMIT, FALSE);
    compressInterval = getConfigInt(CONFIG_CAP_COMPRESS_INTERVAL, FALSE);
    
    logMsg(LOG_DEBUG, "db setup complete");
}

int updateDb(int dr, struct Data* diffList){
    int status = SUCCESS;
 // Insert all the Data structs into the d/b, stopping if there are any failures
    while (diffList != NULL) {
        status = insertDataPartial(dr, diffList);
        if (status == FAIL){
            break;
        }
        diffList = diffList->next;
    }
    return status;
}

int compressDb(){
    logMsg(LOG_DEBUG, "Starting db compression");
    
    int status;

 // Compresses per-second values that are older than 1 minute into per-minute values
    status = compressDbStage(keepPerSecLimit, POLL_INTERVAL, SECS_PER_MIN, (int(*)(int))getNextMinForTs);
    logMsg(LOG_DEBUG, "After first stage of db compression, result=%d", status);
    if (status == FAIL){
        return FAIL;
    }

 // Compresses per-minute values that are older than 1 day into per-hour values
    status = compressDbStage(keepPerMinLimit, SECS_PER_MIN, SECS_PER_HOUR, (int(*)(int))getNextHourForTs);
    logMsg(LOG_DEBUG, "After second stage of db compression, result=%d", status);
    if (status == FAIL){
        return FAIL;
    } else {
        return SUCCESS;
    }
}

int getNextCompressTime(){
 // This calculates when the next call to compressDb is due.
    return getTime() + compressInterval;
}


static int doInsert(int ts, int dr, int fl, BW_INT vl){
 // Inserts a row with the specified values into the 'data' table
    sqlite3_stmt* stmt = getStmt(SQL_INSERT_INTO_DATA);
    sqlite3_bind_int(stmt,    1, ts);
    sqlite3_bind_int(stmt,    2, dr);
    sqlite3_bind_int64(stmt,  3, vl);
    sqlite3_bind_int(stmt,    4, fl);

    int status;
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE){
        logMsg(LOG_ERR, "doInsert() failed to insert values %d,%d,%d,%llu into db rc=%d error=%s", ts, dr, fl, vl, rc, getDbError());
        status = FAIL;
    } else {
        logMsg(LOG_DEBUG, "doInsert() ok: %d,%d,%d,%llu", ts, dr, fl, vl);//TODO
        status = SUCCESS;
    }
    finishedStmt(stmt);

    return status;
}

static int insertDataPartial(int dr, struct Data* data){
 // Inserts the data for a single Data struct into the 'data' table
    return doInsert(data->ts, dr, data->fl, data->vl);
}

int insertData(struct Data* data){
 // Inserts the data for a single Data struct into the 'data' table
    return doInsert(data->ts, data->dr, data->fl, data->vl);
}

static int doDelete(int ts, int dr){
    int status;
 /* Removes all rows with the specified 'dr' value, and with a 'ts' value less than or equal to the given one. This
    gets called during a compressDb operation, to remove the old rows that have been amalgamated into a single row. */
    sqlite3_stmt* stmt = getStmt(SQL_DELETE_COMPRESSED);
    sqlite3_bind_int(stmt, 1, ts);
    sqlite3_bind_int(stmt, 2, dr);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE){
        logMsg(LOG_ERR, "Failed to delete compressed rows for ts=%d dr=%d rc=%d error=%s", ts, dr, rc, getDbError());
        status = FAIL;
    } else {
        status = SUCCESS;
    }

    finishedStmt(stmt);

    return status;
}

static int doCompress(int ts, int oldDr, int newDr){
 // For each adapter, compresses rows older than 'ts' and with a 'dr' of oldDr, into a single row with 'dr' of newDr.
    logMsg(LOG_DEBUG, "doCompress(%d,%d,%d)", ts, oldDr, newDr);
    
    sqlite3_stmt* stmt = getStmt(SQL_SELECT_FOR_COMPRESS);

    sqlite3_bind_int(stmt, 1, ts);
    sqlite3_bind_int(stmt, 2, oldDr);

    int rc;
    BW_INT total;
    int filter;
    int status = SUCCESS;
    int insertedOk, deletedOk;

    while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
     // We get 1 row back for each filter
        filter = sqlite3_column_int(stmt,   0);
        total  = sqlite3_column_int64(stmt, 1);

        logMsg(LOG_DEBUG, "doCompress loop: vl=%llu fl=%d", total, filter);
 
        insertedOk = doInsert(ts, newDr, filter, total);
        if (!insertedOk){
            status = FAIL;
            break;
        }
    }
    if (rc != SQLITE_DONE){
        logMsg(LOG_ERR, "sqlite3_step in doCompress returned %d", rc);
        status = FAIL;   
    }
    finishedStmt(stmt);
    
 // Only remove the old rows if we succeeded in inserting the new one    
    if (status == SUCCESS){
        status = doDelete(ts, oldDr);
    }
    
    return status;
}

static int getMinTs(int dr){
 // Find the smallest 'ts' in the data table having the specified 'dr'
    sqlite3_stmt* stmt = getStmt(SQL_SELECT_MIN_TS);
    sqlite3_bind_int(stmt, 1, dr);

    int minTs;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW){
        minTs = sqlite3_column_int(stmt, 0);
    } else {
     // We found no rows with the specified 'dr' value
        minTs = 0;
    }

    finishedStmt(stmt);

    return minTs;
}

static int compressDbStage(int secKeepInterval, int oldDr, int newDr, int (*fnRoundUp)(int) ){
 // Performs a single stage of compression process, amalgamating old rows with a given 'dr' value into newer rows with a larger 'dr'

 // Calculate the largest 'ts' value that we will compress
    int keepBoundary = getTime() - secKeepInterval;
    int minTsRoundedUp = 0;
    
 // Find the smallest 'ts' value in the 'data' table (having the 'dr' value that we are interested in)
    int minTs = getMinTs(oldDr);
    if (minTs != 0){
     // Round the smallest 'ts' value up to the nearest sensible interval (minute/hour)
        minTsRoundedUp = (*fnRoundUp)(minTs);
    }
    
    int status = SUCCESS;
    int compressOk;

    logMsg(LOG_DEBUG, "In compressDbStage(%d,%d,%d), keep=%d, minTs=%d, minRounded=%d",
            secKeepInterval, oldDr, newDr, keepBoundary, minTs, minTsRoundedUp);

 // Make sure the compression operation is atomic (this includes the deletion of old rows)
    beginTrans(FALSE);
    while((minTs != 0) && (minTsRoundedUp <= keepBoundary)){
        logMsg(LOG_DEBUG, "compressDbStage loop: keep=%d, minTs=%d, minRounded=%d", keepBoundary, minTs, minTsRoundedUp);
        
     // We have rows that need to be compressed, ie that have the correct 'dr' value and are older than the boundary we established earlier
        compressOk = doCompress(minTsRoundedUp, oldDr, newDr);
        if (!compressOk){
            status = FAIL;
            break;
        }

     // Check what the oldest 'ts' value is now, after the previous compression
        int prevMinTs = minTs;
        minTs = getMinTs(oldDr);
        if (minTs != 0){
            if (minTs <= prevMinTs){
             // Should never happen, but sometimes does - I don't know why. Results in nasty CPU-intensive infinite looping
                status = FAIL;
                logMsg(LOG_ERR, "getMinTs() error: oldDr=%d, prevMinTs=%d, minTs=%d,",
                        oldDr, prevMinTs, minTs);
                break;    
            } 
            minTsRoundedUp = (*fnRoundUp)(minTs);
        }
    }

    if (status == SUCCESS){
        commitTrans();
    } else {
        rollbackTrans();
    }

    return status;
}
