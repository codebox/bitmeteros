#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include "client.h"

/*
Contains thread-safe utility functions used by other client modules.
*/

#define TS_BOUNDS_SQL_ALL    "SELECT MIN(ts), MAX(ts) FROM data2"
#define TS_BOUNDS_SQL_FILTER "SELECT MIN(ts), MAX(ts) FROM data2 WHERE fl=?"
#define MAX_VALUES_SQL       "SELECT MAX(vl) AS vl FROM data2"

static struct ValueBounds* buildTsBounds(sqlite3_stmt* stmtTsBounds);

struct ValueBounds* calcTsBounds(int fl){
 /* Calculate the smallest and largest timestamps in the data table that match the
    specified host/adapter combination. If are found then we return NULL. */
    sqlite3_stmt *stmtTsBounds;
    
    if (fl > 0){
		stmtTsBounds = getStmt(TS_BOUNDS_SQL_FILTER);
		sqlite3_bind_int(stmtTsBounds, 1, fl);
    } else {
		stmtTsBounds = getStmt(TS_BOUNDS_SQL_ALL);    	
    }

    struct ValueBounds* tsBounds = buildTsBounds(stmtTsBounds);

	finishedStmt(stmtTsBounds);

    return tsBounds;    
}

static struct ValueBounds* buildTsBounds(sqlite3_stmt* stmtTsBounds){
	struct ValueBounds* values = NULL;
	int rc = sqlite3_step(stmtTsBounds);

	if (rc == SQLITE_ROW){
		time_t minTs = sqlite3_column_int(stmtTsBounds, 0);
		time_t maxTs = sqlite3_column_int(stmtTsBounds, 1);

        if (maxTs > 0){
         // Need a way to show that the table contains no matches - if maxTs==0 then no rows were found so we will return NULL
            values = malloc(sizeof(struct ValueBounds));
            values->min = minTs;
            values->max = maxTs;
        }
	}

    return values;
}

BW_INT getValueForFilterId(struct Data* data, int filterId) {
	while (data != NULL) {
		if (data->fl == filterId){
			return data->vl;	
		}
		data = data->next;	
	}
	return 0;
}

struct Data* calcMaxValue(){
 // Calculate the largest value that exists in the data table
   	sqlite3_stmt *stmtMaxValue = getStmt(MAX_VALUES_SQL);

	struct Data* result = runSelect(stmtMaxValue);

	finishedStmt(stmtMaxValue);

    return result;
}

void formatAmountByUnits(const BW_INT vl, char* vlTxt, int units){
	switch (units) {
		case UNITS_BYTES:
			sprintf(vlTxt, "%llu", vl);
			break;

		case UNITS_ABBREV:
		case UNITS_FULL:
			formatAmount(vl, TRUE, (units == UNITS_ABBREV), vlTxt);
			break;

		default:
			assert(FALSE); // We validate for bad unit values in the options module
			break;
	}
}

