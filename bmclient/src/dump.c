#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "bmclient.h"

static int maxCallback(void *notUsed, int argc, char **argv, char **azColName);
static int mainCallback(void *notUsed, int argc, char **argv, char **azColName);
extern struct Prefs prefs;
static int maxDl, maxUl;

void doDump(){
	int computeMaxValues = (prefs.units == PREF_UNITS_BYTES);

	if (computeMaxValues){
        beginTrans();
		executeSql("select max(dl), max(ul) from data", maxCallback);
	} else {
		maxDl = maxUl = -1;
	}

	executeSql("select ts,dr,dl,ul,ad from data order by ts desc", mainCallback);

	if (computeMaxValues){
		commitTrans();
	}
}

static int maxCallback(void *notUsed, int argc, char **argv, char **azColName){
	maxDl = argv[0] ? strtoull(argv[0], NULL, 10) : 0;
	maxUl = argv[1] ? strtoull(argv[1], NULL, 10) : 0;

	return 0;
}

static int mainCallback(void *notUsed, int argc, char **argv, char **azColName){
	char* ts = argv[0];
	char* dr = argv[1];
	char* dl = argv[2];
	char* ul = argv[3];

	int dri     = atoi(dr);
	int tsiTo   = atoi(ts);
	int tsiFrom = tsiTo - dri;

	char date[11];
	toDate(date, tsiFrom);

	char timeFrom[9];
	toTime(timeFrom, tsiFrom);

	char timeTo[9];
	toTime(timeTo, tsiTo);

	char* dlTxt = (char*) calloc(20, sizeof(char));
	char* ulTxt = (char*) calloc(20, sizeof(char));
	formatAmounts(strtoull(dl, NULL, 10), strtoull(ul, NULL, 10), dlTxt, ulTxt, prefs.units);

	if (prefs.dumpFormat == PREF_DUMP_FORMAT_CSV){
		printf("%s,%s,%s,%d,%s,%s\n", date, timeFrom, timeTo, dri, dlTxt, ulTxt);

	} else if (prefs.dumpFormat == PREF_DUMP_FORMAT_FIXED_WIDTH){
		int dlWidth, ulWidth;
		switch (prefs.units) {
			case PREF_UNITS_BYTES:
				assert(maxDl >= 0 && maxUl >= 0);
				dlWidth = 1 + (int) log10(maxDl);
				ulWidth = 1 + (int) log10(maxUl);
				break;

			case PREF_UNITS_ABBREV:
				assert(maxDl == -1 && maxUl == -1);
				dlWidth = ulWidth = 9;
				break;

			case PREF_UNITS_FULL:
				assert(maxDl == -1 && maxUl == -1);
				dlWidth = ulWidth = 16;
				break;

			default:
				//TODO
				break;
		}
		printf("%s %s %s %4d %*s %*s\n", date, timeFrom, timeTo, dri, dlWidth, dlTxt, ulWidth, ulTxt);

	}
	return 0;
}
