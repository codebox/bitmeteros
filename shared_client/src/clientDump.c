#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "client.h"

/*
Contains a helper function for use by clients that need to performs database dumps
*/

void getDumpValues(int handle, void (*callback)(int, struct Data*)){
 // The callback function gets invoked once for each row in the 'data2' table
 	sqlite3_stmt *stmt = getStmt("SELECT ts AS ts, dr AS dr, vl AS vl, fl AS fl FROM data2 ORDER BY ts DESC");
    runSelectAndCallback(stmt, callback, handle);
    finishedStmt(stmt);
}
