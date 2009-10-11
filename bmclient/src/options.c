#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmclient.h"

static int setMode(struct Prefs*, char* mode);
static int setUnits(struct Prefs*, char* units);
static int setGroup(struct Prefs*, char* group);
static int setHelp(struct Prefs* );
static int setVersion(struct Prefs* );
static int setDumpFormat(struct Prefs*, char* );
static int setRange(struct Prefs*, char* );
static int setDirection(struct Prefs*, char* );
static int setBarChars(struct Prefs*, char*);
static int setMaxAmount(struct Prefs*, char* );
static int setMonitorType(struct Prefs*, char* );
static time_t makeTsFromRange(char* rangePart);
static time_t adjustForEndOfRange(time_t, int );
static void setErrMsg(struct Prefs *, char*);

int parseArgs(int argc, char **argv, struct Prefs *prefs){
	int opt;
	char OPT_LIST[28];
	sprintf(OPT_LIST, "%c:%c:%c:%c%c%c:%c:%c:%c:%c:%c:",
OPT_MODE, OPT_DUMP_FORMAT, OPT_UNITS, OPT_HELP, OPT_VERSION, OPT_RANGE, OPT_GROUP, OPT_DIRECTION, OPT_BAR_CHARS, OPT_MAX_AMOUNT, OPT_MONITOR_TYPE);

	int optOk = 0;

	if (argc <= 1){
		setErrMsg(prefs, ERR_OPT_NO_ARGS);
	} else {
		while ((opt = getopt(argc, argv, OPT_LIST)) != -1){
			switch (opt){
				case OPT_HELP:
					optOk = setHelp(prefs);
					break;
				case OPT_VERSION:
					optOk = setVersion(prefs);
					break;
				case OPT_MODE:
					optOk = setMode(prefs, optarg);
					break;
				case OPT_DUMP_FORMAT:
					optOk = setDumpFormat(prefs, optarg);
					break;
				case OPT_UNITS:
					optOk = setUnits(prefs, optarg);
					break;
				case OPT_RANGE:
					optOk = setRange(prefs, optarg);
					break;
				case OPT_GROUP:
					optOk = setGroup(prefs, optarg);
					break;
				case OPT_DIRECTION:
					optOk = setDirection(prefs, optarg);
					break;
				case OPT_BAR_CHARS:
					optOk = setBarChars(prefs, optarg);
					break;
				case OPT_MAX_AMOUNT:
					optOk = setMaxAmount(prefs, optarg);
					break;
				case OPT_MONITOR_TYPE:
					optOk = setMonitorType(prefs, optarg);
					break;
				default:
					optOk=0;
					break;
			}
			if (!optOk){
				break;
			}
		}
	}
	return optOk;
}

static void setErrMsg(struct Prefs *prefs, char* msg){
	if (prefs->errorMsg != NULL){
		free(prefs->errorMsg);
	}
	if (msg != NULL){
		prefs->errorMsg = malloc(strlen(msg) + 1);
		strcpy(prefs->errorMsg, msg);
	}
}

static int setMonitorType(struct Prefs *prefs, char* monitorType){
	int ok=1;
	if (strcmp(monitorType, ARG_MONITOR_TYPE_NUMS) == 0) {
		prefs->monitorType = PREF_MONITOR_TYPE_NUMS;
	} else if (strcmp(monitorType, ARG_MONITOR_TYPE_BAR) == 0) {
		prefs->monitorType = PREF_MONITOR_TYPE_BAR;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_MONITOR_TYPE);
		ok=0;
	}
	return ok;
}

static int setDirection(struct Prefs *prefs, char* dirTxt){
	int ok=1;
	if (strcmp(dirTxt, ARG_DIRECTION_DL) == 0) {
		prefs->direction = PREF_DIRECTION_DL;
	} else if (strcmp(dirTxt, ARG_DIRECTION_UL) == 0) {
		prefs->direction = PREF_DIRECTION_UL;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_DIRECTION);
		ok=0;
	}
	return ok;
}

static int setBarChars(struct Prefs *prefs, char* barCharsTxt){
	int barChars = atoi(barCharsTxt);
	if (barChars > 0){
		prefs->barChars = barChars;
		return 1;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_WIDTH);
		return 0;
	}
}

static int setMaxAmount(struct Prefs *prefs, char* maxAmountTxt){
	int maxAmount = atoi(maxAmountTxt);
	if (maxAmount > 0){
		prefs->maxAmount = maxAmount;
		return 1;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_MAX);
		return 0;
	}
}

static int setGroup(struct Prefs *prefs, char* groupTxt){
	int ok=1;
	if (strcmp(groupTxt, ARG_GROUP_HOURS) == 0) {
		prefs->group = PREF_GROUP_HOURS;
	} else if (strcmp(groupTxt, ARG_GROUP_DAYS) == 0) {
		prefs->group = PREF_GROUP_DAYS;
	} else if (strcmp(groupTxt, ARG_GROUP_MONTHS) == 0) {
		prefs->group = PREF_GROUP_MONTHS;
	} else if (strcmp(groupTxt, ARG_GROUP_YEARS) == 0) {
		prefs->group = PREF_GROUP_YEARS;
	} else if (strcmp(groupTxt, ARG_GROUP_TOTAL) == 0) {
		prefs->group = PREF_GROUP_TOTAL;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_GROUP);
		ok=0;
	}
	return ok;
}

static int setRange(struct Prefs *prefs, char* rangeTxt){
	int rangeLen = strlen(rangeTxt);

	if (rangeLen > 21){ // yyyymmddhh-yyyymmddhh
		setErrMsg(prefs, ERR_OPT_BAD_RANGE);
		return 0;

	} else {
		char* hyphen    = strchr(rangeTxt, '-');
		char* rangeFrom = (char*) calloc(11, sizeof(char));
		char* rangeTo   = (char*) calloc(11, sizeof(char));
		if (hyphen==0){
			if (rangeLen > 10){
				setErrMsg(prefs, ERR_OPT_BAD_RANGE);
				return 0;

			} else {
				strncpy(rangeFrom, rangeTxt, 10);
				strncpy(rangeTo, rangeTxt, 10);
			}

		} else {
			int hyphenPos = hyphen - rangeTxt;
			strncpy(rangeFrom, rangeTxt, hyphenPos);
			strncpy(rangeTo, rangeTxt+hyphenPos+1, 10);
		}

		int tsFrom = makeTsFromRange(rangeFrom);
		if (tsFrom < 0){
			setErrMsg(prefs, ERR_OPT_BAD_RANGE);
			return 0;
		}

		int tsTo = makeTsFromRange(rangeTo);
		if (tsTo < 0){
			setErrMsg(prefs, ERR_OPT_BAD_RANGE);
			return 0;
		}

		if (tsFrom>tsTo){
			prefs->rangeFrom = tsTo;
			prefs->rangeTo   = adjustForEndOfRange(tsFrom, strlen(rangeFrom));
		} else {
			prefs->rangeFrom = tsFrom;
			prefs->rangeTo   = adjustForEndOfRange(tsTo, strlen(rangeTo));
		}
	}
	return 1;
}

static time_t makeTsFromRange(char* rangePart){
	char* yTxt = "0";
	char* mTxt = "1";
	char* dTxt = "1";
	char* hTxt = "0";

	int rangeLen = strlen(rangePart);
	switch(rangeLen){
		case 10:	// yyyymmddhh
			hTxt = (char*) calloc(3, sizeof(char));
			strncpy(hTxt, rangePart+8, 2);
		 // fall-through

		case 8:		// yyyymmdd
			dTxt = (char*) calloc(3, sizeof(char));
			strncpy(dTxt, rangePart+6, 2);
		 // fall-through

		case 6:		// yyyymm
			mTxt = (char*) calloc(3, sizeof(char));
			strncpy(mTxt, rangePart+4, 2);
		 // fall-through

		case 4:		// yyyy
			yTxt = (char*) calloc(5, sizeof(char));
			strncpy(yTxt, rangePart, 4);
			break;

		default:
			return -1;
			break;
	}

	struct tm cal = {0,0,0,0,0,0,0,0,-1};
	cal.tm_hour = atoi(hTxt);
	cal.tm_mday = atoi(dTxt);
	cal.tm_mon  = atoi(mTxt) - 1;
	cal.tm_year = atoi(yTxt) - 1900;

	return mktime(&cal);
}
static time_t adjustForEndOfRange(time_t time, int rangeLen){
    struct tm* cal = gmtime(&time);
	switch(rangeLen){
		case 10:	// yyyymmddhh
			cal->tm_hour++;
			break;

		case 8:		// yyyymmdd
			cal->tm_mday++;
			break;

		case 6:		// yyyymm
			cal->tm_mon++;
			break;

		case 4:		// yyyy
			cal->tm_year++;
			break;

		default:
			//TODO bad len
			break;
	}

	return mktime(cal);
}

static int setHelp(struct Prefs *prefs){
	prefs->help = 1;
	return 1;
}

static int setVersion(struct Prefs *prefs){
	prefs->version = 1;
	return 1;
}

static int setUnits(struct Prefs *prefs, char* units){
	int ok=1;
	if (strcmp(units, ARG_UNITS_BYTES) == 0) {
		prefs->units = PREF_UNITS_BYTES;
	} else if (strcmp(units, ARG_UNITS_ABBREV) == 0) {
		prefs->units = PREF_UNITS_ABBREV;
	} else if (strcmp(units, ARG_UNITS_FULL) == 0) {
		prefs->units = PREF_UNITS_FULL;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_UNIT);
		ok=0;
	}
	return ok;
}

static int setDumpFormat(struct Prefs *prefs, char* dumpFormat){
	int ok = 1;
	if (strcmp(dumpFormat, ARG_DUMP_FORMAT_CSV_SHORT) == 0 || strcmp(dumpFormat, ARG_DUMP_FORMAT_CSV_LONG) == 0){
		prefs->dumpFormat = PREF_DUMP_FORMAT_CSV;
	} else if (strcmp(dumpFormat, ARG_DUMP_FORMAT_FIXED_WIDTH_SHORT) == 0 || strcmp(dumpFormat, ARG_DUMP_FORMAT_FIXED_WIDTH_LONG) == 0){
		prefs->dumpFormat = PREF_DUMP_FORMAT_FIXED_WIDTH;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_DUMP_FORMAT);
		ok = 0;
	}
	return ok;
}

static int setMode(struct Prefs *prefs, char* mode){
	int ok=1;
	if (strcmp(mode, ARG_MODE_DUMP_SHORT) == 0 || strcmp(mode, ARG_MODE_DUMP_LONG) == 0){
		prefs->mode = PREF_MODE_DUMP;
	} else if (strcmp(mode, ARG_MODE_SUMMARY_SHORT) == 0 || strcmp(mode, ARG_MODE_SUMMARY_LONG) == 0){
		prefs->mode = PREF_MODE_SUMMARY;
	} else if (strcmp(mode, ARG_MODE_MONITOR_SHORT) == 0 || strcmp(mode, ARG_MODE_MONITOR_LONG) == 0){
		prefs->mode = PREF_MODE_MONITOR;
	} else if (strcmp(mode, ARG_MODE_QUERY_SHORT) == 0 || strcmp(mode, ARG_MODE_QUERY_LONG) == 0){
		prefs->mode = PREF_MODE_QUERY;
	} else {
		setErrMsg(prefs, ERR_OPT_BAD_MODE);
		ok=0;
	}
	return ok;
}
