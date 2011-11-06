#include <sqlite3.h>
#include <string.h>
#include "common.h"

/*
Contains a helper function for use by clients that need to monitor the database.
*/

#define CLIENT_MONITOR_SQL     "SELECT ts AS ts, dr AS dr, SUM(vl) AS vl, fl AS fl FROM data WHERE fl = ? GROUP BY ts HAVING ts >= ? ORDER BY ts DESC"
#define CLIENT_MONITOR_SQL_ALL "SELECT ts AS ts, dr AS dr, SUM(vl) AS vl, fl AS fl FROM data GROUP BY ts,fl HAVING ts >= ? ORDER BY ts DESC"

struct Data* getMonitorValues(int ts, int fl){
 // A list of Data structs will be returned, once for each db entry with a timestamp >= ts and a matching filter id
    sqlite3_stmt *stmt;
    if (fl > 0){
        stmt = getStmt(CLIENT_MONITOR_SQL); 
        sqlite3_bind_int(stmt, 1, fl);
        sqlite3_bind_int(stmt, 2, ts);
    } else {
        stmt = getStmt(CLIENT_MONITOR_SQL_ALL);
        sqlite3_bind_int(stmt, 1, ts);
    }

    struct Data* result = runSelect(stmt);

    finishedStmt(stmt);

    return result;
}
