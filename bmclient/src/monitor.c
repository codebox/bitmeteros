#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sqlite3.h>
#include "common.h"
#include "bmclient.h"

static void sigIntHandler();
static void getValues(struct BwValues*, int);
static int stopNow=0;
static sqlite3_stmt *stmtGetValues;
extern struct Prefs prefs;

void doMonitor(){
	prepareSql(&stmtGetValues, "select sum(dl), sum(ul) from data where ts=?");
	signal(SIGINT, sigIntHandler);

	printf("Monitoring... (Ctrl-C to abort)\n");
	struct BwValues values;
	int doBars = (prefs.monitorType == PREF_MONITOR_TYPE_BAR);
	unsigned long amt;
	char amtTxt[20];
	while(!stopNow){
		getValues(&values, getTime()-1);
		if (doBars){
			amt = ((prefs.direction == PREF_DIRECTION_DL) ? values.dl : values.ul);
			formatAmount(amt, 1, 1, amtTxt);
			int i;
			int charCount = prefs.barChars * ((float)amt / prefs.maxAmount);
			charCount = (charCount > prefs.barChars) ? prefs.barChars : charCount;
			printf("%10s|", amtTxt);
			for(i=1; i<= charCount; i++){
				printf("#");
			}
			printf("\n");
		} else {
			printf("DL: %llu UL: %llu\n", values.dl, values.ul);
		}
	    doSleep(1);
	}
	printf("monitoring aborted.\n");
}

static void getValues(struct BwValues* values, int ts){
	sqlite3_bind_int(stmtGetValues, 1, ts);

	int rc = sqlite3_step(stmtGetValues);
	if (rc == SQLITE_ROW){
		values->dl = sqlite3_column_int64(stmtGetValues, 0);
		values->ul = sqlite3_column_int64(stmtGetValues, 1);
	} else {
		//TODO warn
		values->dl = values->ul = 0;
	}
	sqlite3_reset(stmtGetValues);
}

static void sigIntHandler(){
	stopNow = 1;
}
