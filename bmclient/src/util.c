#include <stdio.h>
#include "bmclient.h"
#include "common.h"
extern struct Prefs prefs;

void formatAmounts(const unsigned long long dl, const unsigned long long ul, char* dlTxt, char *ulTxt, int units){
	switch (units) {
		case PREF_UNITS_BYTES:
			sprintf(dlTxt, "%llu", dl);
			sprintf(ulTxt, "%llu", ul);
			break;

		case PREF_UNITS_ABBREV:
		case PREF_UNITS_FULL:
			formatAmount(dl, 0, (prefs.units == PREF_UNITS_ABBREV), dlTxt);
			formatAmount(ul, 0, (prefs.units == PREF_UNITS_ABBREV), ulTxt);
			break;

		default:
			//TODO error
			break;
	}
}

struct ValuesBounds calcTsBounds(){
	sqlite3_stmt *stmtTsBounds;
	prepareSql(&stmtTsBounds, "select min(ts), max(ts) from data");

	int minTs, maxTs;
	int rc = sqlite3_step(stmtTsBounds);
	if (rc == SQLITE_ROW){
		minTs = sqlite3_column_int(stmtTsBounds, 0);
		maxTs = sqlite3_column_int(stmtTsBounds, 1);
	} else {
		maxTs = minTs = 0;
	}
	sqlite3_reset(stmtTsBounds);

	struct ValuesBounds values;
	values.min = minTs;
	values.max = maxTs;
	return values;
}
