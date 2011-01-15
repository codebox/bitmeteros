/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "capture.h"
#include <sqlite3.h>
#include "common.h"

/*
Contains code that interacts with the database on behalf of the BitMeter Data Capture application. At a high level there
are two database operations that are performed:
  - Inserting new rows into the 'data' table (see updateDb)
  - Compressing the 'data' table by amalgamating multiple older rows with small 'dr' values into a single row with a
    larger 'dr' value (see compressDb). This compression is performed at regular intervals, and also when the application
    starts up.
*/

static sqlite3_stmt *stmtInsertData, *stmtSelectForCompression, *stmtSelectMinTsForDr, *stmtDeleteCompressed;

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
	
 // Initialise things, this must be called first
	prepareSql(&stmtInsertData,           "INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (?,?,?,?,?,?)");
	prepareSql(&stmtSelectMinTsForDr,     "SELECT MIN(ts) FROM data WHERE dr=?");
	prepareSql(&stmtSelectForCompression, "SELECT ad, hs, SUM(dl), SUM(ul) FROM (SELECT * FROM data WHERE ts<=? AND dr=?) GROUP BY ad, hs;");
	prepareSql(&stmtDeleteCompressed,     "DELETE FROM data WHERE ts<=? AND dr=?");

 // Read various values out of the 'config' table
	keepPerSecLimit  = getConfigInt("cap.keep_sec_limit", FALSE);
	keepPerMinLimit  = getConfigInt("cap.keep_min_limit", FALSE);
	compressInterval = getConfigInt("cap.compress_interval", FALSE);
	
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


static int doInsert(int ts, int dr, const char* addr, BW_INT dl, BW_INT ul, const char* host){
 // Inserts a row with the specified values into the 'data' table
	sqlite3_bind_int(stmtInsertData,  1, ts);
	sqlite3_bind_int(stmtInsertData,  2, dr);
	if (addr != NULL){
        sqlite3_bind_text(stmtInsertData, 3, addr, strlen(addr), SQLITE_TRANSIENT);
	} else {
        sqlite3_bind_null(stmtInsertData, 3);
	}
	sqlite3_bind_int64(stmtInsertData,  4, dl);
  	sqlite3_bind_int64(stmtInsertData,  5, ul);
  	if (host != NULL){
        sqlite3_bind_text(stmtInsertData, 6, host, strlen(host), SQLITE_TRANSIENT);
  	} else {
  	    sqlite3_bind_null(stmtInsertData, 6);
  	}

	int status;
  	int rc = sqlite3_step(stmtInsertData);
  	if (rc != SQLITE_DONE){
  		logMsg(LOG_ERR, "doInsert() failed to insert values %d,%d,%s,%llu,%llu,%s into db rc=%d error=%s", ts, dr, addr, dl, ul, host, rc, getDbError());
  		status = FAIL;
  	} else {
  		logMsg(LOG_INFO, "doInsert() ok: %d,%d,%s,%llu,%llu,%s", ts, dr, addr, dl, ul, host);//TODO
        status = SUCCESS;
  	}
  	sqlite3_reset(stmtInsertData);

  	return status;
}

static int insertDataPartial(int dr, struct Data* data){
 // Inserts the data for a single Data struct into the 'data' table
	return doInsert(data->ts, dr, data->ad, data->dl, data->ul, data->hs);
}

int insertData(struct Data* data){
 // Inserts the data for a single Data struct into the 'data' table
	return doInsert(data->ts, data->dr, data->ad, data->dl, data->ul, data->hs);
}

static int doDelete(int ts, int dr){
    int status;
 /* Removes all rows with the specified 'dr' value, and with a 'ts' value less than or equal to the given one. This
    gets called during a compressDb operation, to remove the old rows that have been amalgamated into a single row. */
	sqlite3_bind_int(stmtDeleteCompressed, 1, ts);
	sqlite3_bind_int(stmtDeleteCompressed, 2, dr);

	int rc = sqlite3_step(stmtDeleteCompressed);
	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, "Failed to delete compressed rows for ts=%d dr=%d rc=%d error=%s", ts, dr, rc, getDbError());
		status = FAIL;
	} else {
        status = SUCCESS;
	}

	sqlite3_reset(stmtDeleteCompressed);

	return status;
}

static int doCompress(int ts, int oldDr, int newDr){
 // For each adapter, compresses rows older than 'ts' and with a 'dr' of oldDr, into a single row with 'dr' of newDr.
	logMsg(LOG_DEBUG, "doCompress(%d,%d,%d)", ts, oldDr, newDr);

	sqlite3_bind_int(stmtSelectForCompression, 1, ts);
	sqlite3_bind_int(stmtSelectForCompression, 2, oldDr);

	int rc;
	BW_INT dlTotal, ulTotal;
	const void *addr;
	const void *host;
    int status = SUCCESS;
    int insertedOk, deletedOk;

	while((rc=sqlite3_step(stmtSelectForCompression)) == SQLITE_ROW){
     // We get 1 row back for each network adapter/host combination
		addr     = sqlite3_column_text(stmtSelectForCompression,  0);
		host     = sqlite3_column_text(stmtSelectForCompression,  1);
		dlTotal  = sqlite3_column_int64(stmtSelectForCompression, 2);
		ulTotal  = sqlite3_column_int64(stmtSelectForCompression, 3);

		logMsg(LOG_DEBUG, "doCompress loop: dl=%llu ul=%llu addr=%s", dlTotal, ulTotal, addr);

		insertedOk = doInsert(ts, newDr, addr, dlTotal, ulTotal, host);
		if (insertedOk){
         // Only remove the old rows if we succeeded in inserting the new one
			deletedOk = doDelete(ts, oldDr);
			if (!deletedOk){
                status = FAIL;
                break;
			}
		} else {
            status = FAIL;
            break;
		}
	}

  	sqlite3_reset(stmtSelectForCompression);

  	return status;
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

static int compressDbStage(int secKeepInterval, int oldDr, int newDr, int (*fnRoundUp)(int) ){
 // Performs a single stage of compression process, amalgamating old rows with a given 'dr' value into newer rows with a larger 'dr'

 // Calculate the largest 'ts' value that we will compress
	int keepBoundary = getTime() - secKeepInterval;

 // Find the smallest 'ts' value in the 'data' table (having the 'dr' value that we are interested in)
	int minTs = getMinTs(oldDr);

 // Round the smallest 'ts' value up to the nearest sensible interval (minute/hour)
	int minTsRoundedUp = (*fnRoundUp)(minTs);

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
        minTs = getMinTs(oldDr);
        minTsRoundedUp = (*fnRoundUp)(minTs);
    }

    if (status == SUCCESS){
        commitTrans();
    } else {
        rollbackTrans();
    }

    return status;
}
