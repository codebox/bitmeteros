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

#define _GNU_SOURCE
#include "common.h"
#include "capture.h"
#include "CuTest.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define BUFFER_SIZE 2048

static int now;
sqlite3_stmt *selectAllStmt;
static struct Data* storedData;

void getDbPath(char* path){
    strcpy(path, IN_MEMORY_DB);
}

void doSleep(int interval){

}
void getLogPath(char* path){
    strcpy(path, "");
}
void getWebRootPath(char* path){
    strcpy(path, "");
}
#ifdef _WIN32
	void logWin32ErrMsg(char* msg, int rc) {
	}
#endif
void getWebRoot(char* path){
    strcpy(path, "");
}
void setTime(time_t newTime){
    now = newTime;
}
time_t getTime(){
    return now;
}
void populateConfigTable(){
    executeSql("insert into config (key,value) values ('cap.compress_interval',  3600)", NULL);
    executeSql("insert into config (key,value) values ('cap.keep_sec_limit',     3600)", NULL);
    executeSql("insert into config (key,value) values ('cap.keep_min_limit',     86400)", NULL);
    executeSql("insert into config (key,value) values ('cap.busy_wait_interval', 60000)", NULL);
}

static char* originalTz;
void setTzToGmt(){
	#ifndef _WIN32
	 // Need this because the date/time fns use localtime	
		originalTz = getenv("TZ");
	    putenv("TZ=GMT"); 
	#endif
}
void restoreTz(){
	#ifndef _WIN32	
		char restoreTz[32];
	    sprintf(restoreTz, "TZ=%s", originalTz);
	    putenv(restoreTz);
	#endif
}

sqlite3* db;
sqlite3* getDb(){
    return db;
}
void setUpDbForTest(){
    db = openDb();
    executeSql("create table config (key, value)", NULL);
    populateConfigTable();
    executeSql("create table data (ts,dr,ad,dl,ul,hs)", NULL);
    executeSql("create table alert (id, name, active, bound, direction, amount);", NULL);
    executeSql("create table interval (id, yr, mn, dy, wk, hr);", NULL);
    executeSql("create table alert_interval (alert_id, interval_id);", NULL);
}
void setup(){
    setUpDbForTest();
    setupDb();
    setTzToGmt();
    prepareSql(&selectAllStmt, "SELECT ts AS ts, dr AS dr, dl As dl, ul AS ul, ad AS ad, hs AS hs FROM data ORDER BY ts DESC, ad ASC");
}
void emptyDb(){
    executeSql("delete from data", NULL);
    executeSql("delete from alert;", NULL);
    executeSql("delete from alert_interval;", NULL);
    executeSql("delete from interval;", NULL);
}
void addDbRow(time_t ts, int dr, char* ad, int dl, int ul, char* hs){
    char sql[200];
    if (hs != NULL){
        sprintf(sql, "INSERT INTO data (ts, dr, ad, dl, ul, hs) values (%d, %d, '%s', %d, %d, '%s')", (int)ts, dr, ad, dl, ul, hs);
    } else {
        sprintf(sql, "INSERT INTO data (ts, dr, ad, dl, ul) values (%d, %d, '%s', %d, %d)", (int)ts, dr, ad, dl, ul);
    }
    executeSql(sql, NULL);
}
void addDbRowBinaryAddress(time_t ts, int dr, int dl, int ul, char* ad, int adLen){
    char sql[200];
    sprintf(sql, "INSERT INTO data (ts, dr, ad, dl, ul) values (%d, %d, ?, %d, %d)", (int)ts, dr, dl, ul);
    sqlite3_stmt *stmt;
    prepareSql(&stmt, sql);
    sqlite3_bind_blob(stmt, 1, ad, adLen, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, "addDbRowBinaryAddress caused sqlite3_step to return %d.", rc);
	}
	sqlite3_reset(stmt);
}
int tableExists(char* tableName){
	char sql[100];
	sprintf(sql, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'", tableName);
	return getRowCount(sql);
}
int getRowCount(char* sql){
	sqlite3_stmt *stmt;
	prepareSql(&stmt, sql);
	
	int rc, counter=0;
	
	while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
		counter++;
	}
	return counter;
}
void rmConfigRow(char* key){
    char sql[200];
    sprintf(sql, "DELETE FROM config WHERE key='%s'", key);
    executeSql(sql, NULL);
}

void addConfigRow(char* key, char* value){
    rmConfigRow(key);
    
    char sql[200];
    sprintf(sql, "INSERT INTO config values ('%s', '%s')", key, value);
    executeSql(sql, NULL);
}

time_t makeTs(const char* dateTxt){
    struct tm t;
    t.tm_isdst = -1;
    strptime(dateTxt, "%Y-%m-%d %H:%M:%S", &t);
    return mktime(&t);
}
time_t makeTsUtc(const char* dateTxt){
    struct tm t;
    t.tm_isdst = 0;
    char dateTxtGmt[32];
    strcpy(dateTxtGmt, dateTxt);
    strcat(dateTxtGmt, " GMT");
    strptime(dateTxtGmt, "%Y-%m-%d %H:%M:%S %Z", &t);
    return mktime(&t);
}

void checkData(CuTest *tc, struct Data* data, time_t ts, int dr, char* ad, int dl, int ul, char* hs) {
    if (data == NULL){
        CuFail(tc, "struct Data* was NULL");
    } else {
        CuAssertIntEquals(tc, ts,  data->ts);
        CuAssertIntEquals(tc, dr,  data->dr);
        if (ad == NULL){
            CuAssertTrue(tc, data->ad == NULL);
        } else {
            CuAssertStrEquals(tc, ad,  data->ad);
        }
        if (hs == NULL){
            CuAssertTrue(tc, data->hs == NULL);
        } else {
            CuAssertStrEquals(tc, hs,  data->hs);
        }
        CuAssertIntEquals(tc, (BW_INT)dl,  data->dl);
        CuAssertIntEquals(tc, (BW_INT)ul,  data->ul);
    }
}

static FILE* tmpFile = NULL;
char* tmpFileData;
FILE* makeTmpFileStream(){
    if (tmpFile != NULL){
        fclose(tmpFile);
        tmpFile = NULL;
        free(tmpFileData);
    }
    tmpFile = tmpfile();
    return tmpFile;
}
int makeTmpFile(){
    return fileno( makeTmpFileStream());
}
char* readTmpFile(){
    assert(tmpFile != NULL);
    rewind(tmpFile);
    tmpFileData = malloc(BUFFER_SIZE);
    int l = fread(tmpFileData, 1, BUFFER_SIZE, tmpFile);
    tmpFileData[l] = 0;
    return tmpFileData;
}

void parseCommandLine(char* cmdLineTxt, char*** argv, int* argc){
    int count = 1;

	char *cmdLineCopy = strdupa(cmdLineTxt);
	char* match = strtok(cmdLineCopy, " ");
 // First, count the number of arguments
	while(match != NULL){
        count++;
        match = strtok(NULL, " ");
	}

    *argv = malloc(sizeof(char*) * (count+1));
    (*argv)[count] = 0;
    **argv = strdup("test"); // the program name would normally go in here

    if (count > 1){
        int i=1;
        cmdLineCopy = strdupa(cmdLineTxt);
        match = strtok(cmdLineCopy, " ");
        do{
            (*argv)[i++] = strdup(match);
            match = strtok(NULL, " ");
        } while(match != NULL);
    }

    *argc = count;
}

void checkDateCriteriaPart(CuTest *tc, struct DateCriteriaPart* part, int isRelative, int val1, int val2, int next){
    CuAssertIntEquals(tc, part->isRelative, isRelative);
    CuAssertIntEquals(tc, part->val1, val1);
    CuAssertIntEquals(tc, part->val2, val2);
    if (next){
        CuAssertTrue(tc, NULL != (part->next));
    } else {
        CuAssertTrue(tc, NULL == (part->next));
    }
}

static void printTmStruct(struct tm tm){
	printf("%d %d %d (%d) %d:%d:%d\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_wday, 
			tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static struct Data* nextData = NULL;
struct Data* getData(){
	return nextData;
}
void setData(struct Data* data){
	nextData = data;
}

static void cbAppendData(int ignored, struct Data* data){
	appendData(&storedData, data);
}

void checkTableContents(CuTest *tc, int rowCount, ...){
 // Helper function for checking the contents of the database
    va_list ap;
    va_start(ap,rowCount);
    storedData = NULL;
    runSelectAndCallback(selectAllStmt, &cbAppendData, 0);
	sqlite3_reset(selectAllStmt);

    struct Data expected;
    struct Data* pStored   = storedData;

    int i;
    for(i=0; i<rowCount; i++){
        expected = va_arg(ap, struct Data);
        CuAssertTrue(tc, pStored != NULL);
        CuAssertIntEquals(tc, expected.ts, pStored->ts);
        CuAssertIntEquals(tc, expected.dr, pStored->dr);
        CuAssertIntEquals(tc, expected.dl, pStored->dl);
        CuAssertIntEquals(tc, expected.ul, pStored->ul);
        CuAssertStrEquals(tc, expected.ad, pStored->ad);
        CuAssertStrEquals(tc, expected.hs, pStored->hs);

        pStored   = pStored->next;
    }
    va_end(ap);

    CuAssertTrue(tc, pStored == NULL);
}


