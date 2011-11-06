#define _GNU_SOURCE
#include <stdlib.h>
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "capture.h"
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "test.h"

#define MAX_MSG_SIZE 512
#define BUFFER_SIZE 2048

void* _test_strdup(char* s, const char* file, const int line){
    char *result = (char*) _test_malloc(strlen(s) + 1, __FILE__, __LINE__);
    if (result == (char*)0){
        return (char*) 0;
    } else {
        strcpy(result, s);
        return result;
    }
}

void printf_output(char* msg){
    check_expected(msg);    
}

void _test_printf(char* fmt, ...){
    va_list argp;
    va_start(argp, fmt);
    
    char msg[MAX_MSG_SIZE];
    vsprintf(&msg, fmt, argp);
    printf_output(msg); 
    va_end(argp);   
}
void _test_printOut(int colour, char* fmt, ...){
    va_list argp;
    va_start(argp, fmt);
    
    char msg[MAX_MSG_SIZE];
    vsprintf(&msg, fmt, argp);
    printf_output(msg); 
    va_end(argp);   
}
void checkDateCriteriaPart(struct DateCriteriaPart* part, int isRelative, int val1, int val2, int next){
    assert_int_equal(part->isRelative, isRelative);
    assert_int_equal(part->val1, val1);
    assert_int_equal(part->val2, val2);
    if (next){
        assert_true(NULL != (part->next));
    } else {
        assert_true(NULL == (part->next));
    }
}

time_t makeTs(const char* dateTxt){
    struct tm t;
    t.tm_isdst = -1;
    strptime(dateTxt, "%Y-%m-%d %H:%M:%S", &t);
    return mktime(&t);
}

void checkData(struct Data* data, time_t ts, int dr, int vl, int fl) {
    if (data == NULL){
        fail();
    } else {
        assert_int_equal(ts,  data->ts);
        assert_int_equal(dr,  data->dr);
        assert_int_equal(fl,  data->fl);
        assert_int_equal((BW_INT)vl,  data->vl);
    }
}

void checkFilter(struct Filter* filter, int id, char* desc, char* name, char* expr, char* host){
    if (filter == NULL){
        fail(); 
    } else {
        assert_int_equal(id, filter->id);
        assert_string_equal(desc, filter->desc);    
        assert_string_equal(name, filter->name);
        assert_string_equal(expr, filter->expr);
        if (host == NULL){
            assert_true(filter->host == NULL);  
        } else {
            assert_string_equal(host, filter->host);
        }
    }
}

void setupTestDb(void** state){
    openDb();
    executeSql("create table config (key, value)", NULL);
    //executeSql("insert into config (key,value) values ('cap.compress_interval',  3600)", NULL);
    //executeSql("insert into config (key,value) values ('cap.keep_sec_limit',     3600)", NULL);
    //executeSql("insert into config (key,value) values ('cap.keep_min_limit',     86400)", NULL);
    //executeSql("insert into config (key,value) values ('cap.busy_wait_interval', 60000)", NULL);
    executeSql("create table data (ts,dr,vl,fl)", NULL);
    executeSql("create table alert (id, name, active, bound, filter, amount);", NULL);
    executeSql("create table interval (id, yr, mn, dy, wk, hr);", NULL);
    executeSql("create table alert_interval (alert_id, interval_id);", NULL);
    executeSql("create table filter (id INTEGER PRIMARY KEY AUTOINCREMENT, desc, name, expr, host)", NULL);
}

void tearDownTestDb(void** state){
    closeDb();
}

void emptyDb(){
    executeSql("delete from data", NULL);
    executeSql("delete from alert;", NULL);
    executeSql("delete from alert_interval;", NULL);
    executeSql("delete from interval;", NULL);
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

void addDbRow(time_t ts, int dr, int vl, int fl){
    char sql[200];
    sprintf(sql, "INSERT INTO data (ts, dr, vl, fl) values (%d, %d, %d, %d)", (int)ts, dr, vl, fl);
    executeSql(sql, NULL);
}

void addFilterRow(int id, char* desc, char* name, char* expr, char* host){
    char sql[200];
    if (host == NULL){
        sprintf(sql, "INSERT INTO filter (id, desc, name, expr) values (%d, '%s', '%s', '%s')", id, desc, name, expr);
    } else {
        sprintf(sql, "INSERT INTO filter (id, desc, name, expr, host) values (%d, '%s', '%s', '%s', '%s')", id, desc, name, expr, host);
    }
    executeSql(sql, NULL);
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

void checkTableContents(struct Data* expectedData){
    sqlite3_stmt* stmt = getStmt("SELECT ts AS ts, dr AS dr, vl As vl, fl AS fl FROM data ORDER BY ts DESC, fl ASC");

    int rc;
    while((rc=sqlite3_step(stmt)) == SQLITE_ROW){
        assert_true(expectedData != NULL);
        assert_int_equal(expectedData->ts, sqlite3_column_int(stmt, 0));
        assert_int_equal(expectedData->dr, sqlite3_column_int(stmt, 1));
        assert_int_equal(expectedData->vl, sqlite3_column_int64(stmt, 2));
        assert_int_equal(expectedData->fl, sqlite3_column_int(stmt, 3));
        expectedData = expectedData->next;
    }

    finishedStmt(stmt);
}

void dumpDataTable(){
    sqlite3_stmt* stmt = getStmt("select ts as ts, dr as dr, vl as vl, fl as fl from data");
    
    dbg("==================\n");
    while(sqlite3_step(stmt) == SQLITE_ROW){
        dbg("%d %d %llu %d\n", sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), sqlite3_column_int64(stmt, 2), sqlite3_column_int(stmt, 3));
    }
    dbg("==================\n");
    finishedStmt(stmt);
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

int tableHasColumn(char* table, char* column){
    char sql[32];
    sprintf(sql, "pragma table_info(%s)", table);
    sqlite3_stmt* stmt = getStmt(sql);
    
    int found = FALSE;
    while(sqlite3_step(stmt) == SQLITE_ROW){
        if (strcmp(sqlite3_column_text(stmt, 1), column) == 0){
            found = TRUE;
            break;
        }
    }
    finishedStmt(stmt);
    
    return found;
}

int tableExists(char* tableName){
    char sql[100];
    sprintf(sql, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'", tableName);
    return getRowCount(sql);
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
