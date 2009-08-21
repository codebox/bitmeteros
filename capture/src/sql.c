#include <stdio.h>
#include <stdlib.h>
#include "capture.h"
#include <sqlite3.h>
#include "common.h"

/*  Contains code that interacts with the database on behalf of the BitMeter Data Capture application. At a high level there
    are two database operations that are performed:
      - Inserting new rows into the 'data' table (see updateDb)
      - Compressing the 'data' table by amalgamating multiple older rows with small 'dr' values into a single row with a
        larger 'dr' value (see compressDb). This compression is performed at regular intervals, and also when the application
        starts up. */

static sqlite3_stmt *stmtInsertData, *stmtSelectConfig, *stmtSelectForCompression, *stmtSelectMinTsForDr, *stmtDeleteCompressed;

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

static int getConfigInt(const char* key);
static void insertBwData(int ts, int dr, struct BwData* data);
static void compressDbStage(int secKeepInterval, int oldDr, int newDr, int (*fnRoundUp)(int) );

void setupDb(){
 // Initialise things, this must be called first
	prepareSql(&stmtInsertData,           "insert into data (ts,dr,ad,dl,ul) values (?,?,?,?,?)");
	prepareSql(&stmtSelectConfig,         "select value from config where key=?");
	prepareSql(&stmtSelectMinTsForDr,     "select min(ts) from data where dr=?");
	prepareSql(&stmtSelectForCompression, "select ad, sum(dl), sum(ul) from (select * from data where ts<=? and dr=?) group by ad;");
	prepareSql(&stmtDeleteCompressed,     "delete from data where ts<=? and dr=?");

 // Read various values out of the 'config' table
	int busyWaitInterval = getConfigInt("cap.busy_wait_interval");
	setDbBusyWait(busyWaitInterval);

	keepPerSecLimit  = getConfigInt("cap.keep_sec_limit");
	keepPerMinLimit  = getConfigInt("cap.keep_min_limit");
	compressInterval = getConfigInt("cap.compress_interval");
}

void updateDb(int ts, int dr, struct BwData* diffList){
	while (diffList != NULL) {
		insertBwData(ts, dr, diffList);
		diffList = diffList->next;
	}
}

void compressDb(){
 // Compresses per-second values that are older than 1 minute into per-minute values
	compressDbStage(keepPerSecLimit, POLL_INTERVAL, SECS_PER_MIN, (int(*)(int))getNextMinForTs);

 // Compresses per-minute values that are older than 1 day into per-hour values
	compressDbStage(keepPerMinLimit, SECS_PER_MIN, SECS_PER_HOUR, (int(*)(int))getNextHourForTs);
}

int getNextCompressTime(){
 // This calculates when the next call to compressDb is due.
	return getTime() + compressInterval;
}


static int doInsert(int ts, int dr, const void* addr, int addrLen, int dl, int ul){
 // Inserts a row with the specified values into the 'data' table
	sqlite3_bind_int(stmtInsertData,  1, ts);
	sqlite3_bind_int(stmtInsertData,  2, dr);
	sqlite3_bind_blob(stmtInsertData, 3, addr, addrLen, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmtInsertData,  4, dl);
  	sqlite3_bind_int(stmtInsertData,  5, ul);

	int ok = 1;
  	int rc = sqlite3_step(stmtInsertData);
  	if (rc != SQLITE_DONE){
  		logMsg(LOG_ERR, "doInsert() failed to insert values %d,%d,%s,%d,%d into db rc=%d error=%s\n", ts, dr, addr, dl, ul, rc, getDbError());
  		ok = 0;
  	}
  	sqlite3_reset(stmtInsertData);

  	return ok;
}

static void insertBwData(int ts, int dr, struct BwData* data){
 // Inserts the data for a single BwData into the 'data' table
	doInsert(ts, dr, data->addr, data->addrLen, data->dl, data->ul);
}

static void doDelete(int ts, int dr){
 /* Removes all rows with the specified 'dr' value, and with a 'ts' value less than or equal to the given one. This
    gets called during a compressDb operation, to remove the old rows that have been amalgamated into a single row. */
	sqlite3_bind_int(stmtDeleteCompressed, 1, ts);
	sqlite3_bind_int(stmtDeleteCompressed, 2, dr);
	int rc = sqlite3_step(stmtDeleteCompressed);
	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, "Failed to delete compressed rows for ts=%d dr=%d rc=%d error=%s\n", ts, dr, rc, getDbError());
	}
	sqlite3_reset(stmtDeleteCompressed);
}

static void doCompress(int ts, int oldDr, int newDr){
 // For each adapter, compresses rows older than 'ts' and with a 'dr' of oldDr, into a single row with 'dr' of newDr.
	logMsg(LOG_INFO, "doCompress for %d\n", ts);
	sqlite3_bind_int(stmtSelectForCompression, 1, ts);
	sqlite3_bind_int(stmtSelectForCompression, 2, oldDr);

	int rc, dlTotal, ulTotal, addrSize;
	const void *addr;

	while((rc=sqlite3_step(stmtSelectForCompression)) == SQLITE_ROW){
     // We get 1 row back for each network adapter
		addr     = sqlite3_column_blob(stmtSelectForCompression,  0);
		addrSize = sqlite3_column_bytes(stmtSelectForCompression, 0);
		dlTotal  = sqlite3_column_int(stmtSelectForCompression,  1);
		ulTotal  = sqlite3_column_int(stmtSelectForCompression,  2);

		logMsg(LOG_INFO, "row dl=%d ul=%d\n", dlTotal, ulTotal);

		int insertedOk = doInsert(ts, newDr, addr, addrSize, dlTotal, ulTotal);
		if (insertedOk){
         // Only remove the old rows if we succeeded in inserting the new one
			doDelete(ts, oldDr);
		}
	}

  	sqlite3_reset(stmtSelectForCompression);
}

static int getMinTs(int dr){
 // Find the smallest 'ts' in the data table having the specified 'dr'
	sqlite3_bind_int(stmtSelectMinTsForDr, 1, dr);

	int minTs;
	int rc = sqlite3_step(stmtSelectMinTsForDr);
	if (rc == SQLITE_ROW){
		minTs = sqlite3_column_int(stmtSelectMinTsForDr, 0);
	} else {
     // We found no rows with the specified 'dr' value
		minTs = 0;
	}
	sqlite3_reset(stmtSelectMinTsForDr);

	return minTs;
}

static void compressDbStage(int secKeepInterval, int oldDr, int newDr, int (*fnRoundUp)(int) ){
 // Performs a single stage of compression process, amalgamating old rows with a given 'dr' value into newer rows with a larger 'dr'

 // Calculate the largest 'ts' value that we will compress
	int keepBoundary = getTime() - secKeepInterval;

 // Find the smallest 'ts' value in the 'data' table (having the 'dr' value that we are interested in)
	int minTs = getMinTs(oldDr);

 // Round the smallest 'ts' value up to the nearest sensible interval (minute/hour)
	int minTsRoundedUp = (*fnRoundUp)(minTs);

 // Make sure the compression operation is atomic (this includes the deletion of old rows)
	beginTrans();
    while((minTs != 0) && (minTsRoundedUp <= keepBoundary)){
     // We have rows that need to be compressed, ie that have the correct 'dr' value and are older than the boundary we established earlier
        doCompress(minTsRoundedUp, oldDr, newDr);

     // Check what the oldest 'ts' value is now, after the previous compression
        minTs = getMinTs(oldDr);
        minTsRoundedUp = (*fnRoundUp)(minTs);
    }
    commitTrans();
}

static int getConfigInt(const char* key){
 // Return the specified value from the 'config' table
	sqlite3_bind_text(stmtSelectConfig, 1, key, -1, SQLITE_TRANSIENT);
	int rc = sqlite3_step(stmtSelectConfig);

	int value;
	if (rc == SQLITE_ROW){
		value = sqlite3_column_int(stmtSelectConfig, 0);
	} else {
		logMsg(LOG_ERR, "Unable to retrieve config value for '%s' rc=%d error=%s\n", key, rc, getDbError());
		value = -1;
	}

  	sqlite3_reset(stmtSelectConfig);
  	return value;
}
