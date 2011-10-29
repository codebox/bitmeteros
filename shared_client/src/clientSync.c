#include <sqlite3.h>
#include "common.h"

/*
Contains a helper function for use by clients that need to retrieve data to be synchronized with another database.
*/

#define CLIENT_SYNC_SQL "SELECT ts AS ts, dr AS dr, vl AS vl, fl AS fl FROM data, filter WHERE data.ts > ? AND data.fl = filter.id AND filter.host IS NULL ORDER BY data.ts ASC"

struct Data* getSyncValues(time_t ts){
 // A list of Data structs will be returned, once for each db entry with a timestamp > ts

   	sqlite3_stmt *stmt = getStmt(CLIENT_SYNC_SQL);

	sqlite3_bind_int(stmt, 1, ts);
	struct Data* result = runSelect(stmt);

	finishedStmt(stmt);

	return result;
}

