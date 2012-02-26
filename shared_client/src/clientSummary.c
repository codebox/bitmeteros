#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "client.h"

/*
Contains a helper function for use by clients that need to produce database summaries.
*/

#define DATA_SUMMARY_SQL_ALL "SELECT SUM(vl) AS vl, fl AS fl FROM data WHERE ts>=? GROUP BY fl ORDER BY fl"
#define HOST_SUMMARY_SQL     "SELECT DISTINCT(host) AS host FROM filter WHERE host != ''"

static void getHosts(char*** hostNames, int* hostCount);
struct Data* calcTotals(int ts);

struct Summary getSummaryValues(){
 // Populate a Summary struct
    struct Summary summary;
    int now = getTime();

    int tsForStartOfToday = getCurrentLocalDayForTs(now);
    int tsForStartOfMonth = getCurrentLocalMonthForTs(now);
    int tsForStartOfYear  = getCurrentLocalYearForTs(now);

    getHosts(&(summary.hostNames), &(summary.hostCount));

    summary.today = calcTotals(tsForStartOfToday);
    summary.month = calcTotals(tsForStartOfMonth);
    summary.year  = calcTotals(tsForStartOfYear);
    summary.total = calcTotals(0);

    struct ValueBounds* tsBounds = calcTsBounds(0);
    if (tsBounds != NULL){
        summary.tsMin = tsBounds->min;
        summary.tsMax = tsBounds->max;
        free(tsBounds);
    } else {
     // No data found
        summary.tsMin = 0;
        summary.tsMax = 0;
    }

    return summary;
}

void freeSummary(struct Summary* summary){
 // Release all the memory allocated to this struct
    freeData(summary->today);
    freeData(summary->month);
    freeData(summary->year);
    freeData(summary->total);

    if (summary->hostNames != NULL){
        int i;
        for(i=0; i<summary->hostCount; i++){
            free(summary->hostNames[i]);
        }
        free(summary->hostNames);
    }
}

static void getHosts(char*** hostNames, int* hostCount){
 // TODO do this better, runs query twice
    int rc; 
    sqlite3_stmt *stmt;
    
    stmt = getStmt(HOST_SUMMARY_SQL);
    *hostCount = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        (*hostCount)++;
    }
    finishedStmt(stmt);
    
    if (*hostCount > 0){
        *hostNames = malloc(sizeof(char*) * (*hostCount));
        
        int offset = 0;
        stmt = getStmt(HOST_SUMMARY_SQL);
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            (*hostNames)[offset++] = strdup(sqlite3_column_text(stmt, 0));
        }
        finishedStmt(stmt);
    } else {
        *hostNames = NULL;
    }
}

struct Data* calcTotals(int ts){
    sqlite3_stmt *stmt = getStmt(DATA_SUMMARY_SQL_ALL);

    sqlite3_bind_int(stmt, 1, ts);
    struct Data* result = runSelect(stmt);
    finishedStmt(stmt);

    return result;
}
