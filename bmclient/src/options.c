#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmclient.h"

#define DEFAULT_BAR_CHARS  69
#define DEFAULT_MAX_AMOUNT 100000

struct Prefs prefs = {PREF_MODE_SUMMARY, PREF_DUMP_FORMAT_FIXED_WIDTH, PREF_UNITS_BYTES, 0, 0, 0, 0, PREF_GROUP_TOTAL, PREF_DIRECTION_DL, DEFAULT_BAR_CHARS, DEFAULT_MAX_AMOUNT, 0, NULL};


static int setMode(char* mode);
static int setUnits(char* units);
static int setGroup(char* group);
static int setHelp();
static int setVersion();
static int setDumpFormat(char* );
static int setRange(char* );
static int setDirection(char* );
static int setBarChars(char*);
static int setMaxAmount(char* );
static int setMonitorType(char* );
static int makeTsFromRange(char* rangePart, int startOfRange);
static void adjustForEndOfRange(struct tm* cal, int rangeLen);

int parseArgs(int argc, char **argv){
	int opt;
	char OPT_LIST[28];
	sprintf(OPT_LIST, "%c:%c:%c:%c%c%c:%c:%c:%c:%c:%c:", 
OPT_MODE, OPT_DUMP_FORMAT, OPT_UNITS, OPT_HELP, OPT_VERSION, OPT_RANGE, OPT_GROUP, OPT_DIRECTION, OPT_BAR_CHARS, OPT_MAX_AMOUNT, OPT_MONITOR_TYPE);
	
	int optOk = 0;
	
	if (argc <= 1){
		prefs.errorMsg = "No arguments supplied.";
	} else {
		while ((opt = getopt(argc, argv, OPT_LIST)) != -1){
			switch (opt){
				case OPT_HELP:
					optOk = setHelp();
					break;
				case OPT_VERSION:
					optOk = setVersion();
					break;
				case OPT_MODE:
					optOk = setMode(optarg);
					break;
				case OPT_DUMP_FORMAT:
					optOk = setDumpFormat(optarg);
					break;			
				case OPT_UNITS:
					optOk = setUnits(optarg);
					break;	
				case OPT_RANGE:
					optOk = setRange(optarg);
					break;
				case OPT_GROUP:
					optOk = setGroup(optarg);
					break;
				case OPT_DIRECTION:
					optOk = setDirection(optarg);
					break;
				case OPT_BAR_CHARS:
					optOk = setBarChars(optarg);
					break;
				case OPT_MAX_AMOUNT:
					optOk = setMaxAmount(optarg);
					break;
				case OPT_MONITOR_TYPE:
					optOk = setMonitorType(optarg);
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

static int setMonitorType(char* monitorType){
	int ok=1;
	if (strcmp(monitorType, ARG_MONITOR_TYPE_NUMS) == 0) {
		prefs.monitorType = PREF_MONITOR_TYPE_NUMS;
	} else if (strcmp(monitorType, ARG_MONITOR_TYPE_BAR) == 0) {
		prefs.monitorType = PREF_MONITOR_TYPE_BAR;
	} else {
		prefs.errorMsg = "Unrecognised monitor type argument";
		ok=0;
	}
	return ok;
}

static int setDirection(char* dirTxt){
	int ok=1;
	if (strcmp(dirTxt, ARG_DIRECTION_DL) == 0) {
		prefs.direction = PREF_DIRECTION_DL;
	} else if (strcmp(dirTxt, ARG_DIRECTION_UL) == 0) {
		prefs.direction = PREF_DIRECTION_UL;
	} else {
		prefs.errorMsg = "Unrecognised direction argument";
		ok=0;
	}
	return ok;
}

static int setBarChars(char* barCharsTxt){
	int barChars = atoi(barCharsTxt);
	if (barChars > 0){
		prefs.barChars = barChars;
		return 1;
	} else {
		prefs.errorMsg = "Invalid -w argument, must be a number > 0";
		return 0;
	}
}

static int setMaxAmount(char* maxAmountTxt){
	int maxAmount = atoi(maxAmountTxt);
	if (maxAmount > 0){
		prefs.maxAmount = maxAmount;
		return 1;
	} else {
		prefs.errorMsg = "Invalid -x argument, must be a number > 0";
		return 0;
	}
}

#define RANGE_ERR "Invalid range argument, check the Help for acceptable range formats"

static int setGroup(char* groupTxt){
	int ok=1;
	if (strcmp(groupTxt, ARG_GROUP_HOURS) == 0) {
		prefs.group = PREF_GROUP_HOURS;
	} else if (strcmp(groupTxt, ARG_GROUP_DAYS) == 0) {
		prefs.group = PREF_GROUP_DAYS;
	} else if (strcmp(groupTxt, ARG_GROUP_MONTHS) == 0) {
		prefs.group = PREF_GROUP_MONTHS;
	} else if (strcmp(groupTxt, ARG_GROUP_YEARS) == 0) {
		prefs.group = PREF_GROUP_YEARS;
	} else if (strcmp(groupTxt, ARG_GROUP_TOTAL) == 0) {
		prefs.group = PREF_GROUP_TOTAL;
	} else {
		prefs.errorMsg = "Unrecognised group type";
		ok=0;
	}		
	return ok;
}

static int setRange(char* rangeTxt){
	int rangeLen = strlen(rangeTxt);
	
	if (rangeLen > 21){ // yyyymmddhh-yyyymmddhh
		prefs.errorMsg = RANGE_ERR;
		return 0;
		
	} else {
		char* hyphen    = strchr(rangeTxt, '-');
		char* rangeFrom = (char*) calloc(11, sizeof(char));
		char* rangeTo   = (char*) calloc(11, sizeof(char));
		if (hyphen==0){
			if (rangeLen > 10){
				prefs.errorMsg = RANGE_ERR;
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
		
		int tsFrom = makeTsFromRange(rangeFrom, 0);
		if (tsFrom < 0){
			prefs.errorMsg = RANGE_ERR;
			return 0;
		}
		
		int tsTo = makeTsFromRange(rangeTo, 1);
		if (tsTo < 0){
			prefs.errorMsg = RANGE_ERR;
			return 0;
		}
		
		if (tsFrom>tsTo){
			prefs.rangeFrom = tsTo;
			prefs.rangeTo   = tsFrom;
		} else {
			prefs.rangeFrom = tsFrom;
			prefs.rangeTo   = tsTo;
		}
	}
	return 1;
}

static int makeTsFromRange(char* rangePart, int endOfRange){
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

	if (endOfRange==1){
		adjustForEndOfRange(&cal, rangeLen);
	}
	
	int ts = mktime(&cal);
	
	return ts;
}
static void adjustForEndOfRange(struct tm* cal, int rangeLen){
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
}

static int setHelp(){
	prefs.help = 1;
	return 1;
}

static int setVersion(){
	prefs.version = 1;
	return 1;
}

static int setUnits(char* units){
	int ok=1;
	if (strcmp(units, ARG_UNITS_BYTES) == 0) {
		prefs.units = PREF_UNITS_BYTES;
	} else if (strcmp(units, ARG_UNITS_ABBREV) == 0) {
		prefs.units = PREF_UNITS_ABBREV;
	} else if (strcmp(units, ARG_UNITS_FULL) == 0) {
		prefs.units = PREF_UNITS_FULL;
	} else {
		prefs.errorMsg = "Unrecognised unit type";
		ok=0;
	}		
	return ok;
}

static int setDumpFormat(char* dumpFormat){
	int ok = 1;
	if (strcmp(dumpFormat, ARG_DUMP_FORMAT_CSV_SHORT) == 0 || strcmp(dumpFormat, ARG_DUMP_FORMAT_CSV_LONG) == 0){
		prefs.dumpFormat = PREF_DUMP_FORMAT_CSV;
	} else if (strcmp(dumpFormat, ARG_DUMP_FORMAT_FIXED_WIDTH_SHORT) == 0 || strcmp(dumpFormat, ARG_DUMP_FORMAT_FIXED_WIDTH_LONG) == 0){
		prefs.dumpFormat = PREF_DUMP_FORMAT_FIXED_WIDTH;
	} else {
		prefs.errorMsg = "Unrecognised dump format";
		ok = 0;
	}	
	return ok;
}

static int setMode(char* mode){
	int ok=1;
	if (strcmp(mode, ARG_MODE_DUMP_SHORT) == 0 || strcmp(mode, ARG_MODE_DUMP_LONG) == 0){
		prefs.mode = PREF_MODE_DUMP;
	} else if (strcmp(mode, ARG_MODE_SUMMARY_SHORT) == 0 || strcmp(mode, ARG_MODE_SUMMARY_LONG) == 0){
		prefs.mode = PREF_MODE_SUMMARY;
	} else if (strcmp(mode, ARG_MODE_MONITOR_SHORT) == 0 || strcmp(mode, ARG_MODE_MONITOR_LONG) == 0){
		prefs.mode = PREF_MODE_MONITOR;
	} else if (strcmp(mode, ARG_MODE_QUERY_SHORT) == 0 || strcmp(mode, ARG_MODE_QUERY_LONG) == 0){
		prefs.mode = PREF_MODE_QUERY;
	} else {
		prefs.errorMsg = "Unrecognised mode";
		ok=0;
	}
	return ok;
}
