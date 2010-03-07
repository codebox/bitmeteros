/*
 * BitMeterOS v0.3.2
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build Date: Sun, 07 Mar 2010 14:49:47 +0000
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

#define INVALID_TS -1

/*
Parse the bmclient command-line, and populate a Prefs structure to indicate what the user asked for.
*/

int parseArgs(int argc, char **argv, struct Prefs *prefs){
	char OPT_LIST[28];
	sprintf(OPT_LIST, "%c:%c:%c:%c%c%c:%c:%c:%c:%c:%c:",
			OPT_MODE, OPT_DUMP_FORMAT, OPT_UNITS, OPT_HELP, OPT_VERSION,
			OPT_RANGE, OPT_GROUP, OPT_DIRECTION, OPT_BAR_CHARS, OPT_MAX_AMOUNT,
			OPT_MONITOR_TYPE);

	int status = FAIL;

	if (argc <= 1){
	 // The command line was empty
		setErrMsg(prefs, ERR_OPT_NO_ARGS);
	} else {
		int opt;
		while ((opt = getopt(argc, argv, OPT_LIST)) != -1){
			switch (opt){
				case OPT_HELP:
					status = setHelp(prefs);
					break;
				case OPT_VERSION:
					status = setVersion(prefs);
					break;
				case OPT_MODE:
					status = setMode(prefs, optarg);
					break;
				case OPT_DUMP_FORMAT:
					status = setDumpFormat(prefs, optarg);
					break;
				case OPT_UNITS:
					status = setUnits(prefs, optarg);
					break;
				case OPT_RANGE:
					status = setRange(prefs, optarg);
					break;
				case OPT_GROUP:
					status = setGroup(prefs, optarg);
					break;
				case OPT_DIRECTION:
					status = setDirection(prefs, optarg);
					break;
				case OPT_BAR_CHARS:
					status = setBarChars(prefs, optarg);
					break;
				case OPT_MAX_AMOUNT:
					status = setMaxAmount(prefs, optarg);
					break;
				case OPT_MONITOR_TYPE:
					status = setMonitorType(prefs, optarg);
					break;
				default:
					status = FAIL;
					break;
			}
		 // Check the 'status' value after each option, and stop if we see anything invalid
			if (status == FAIL){
				break;
			}
		}
	}
	return status;
}

static void setErrMsg(struct Prefs *prefs, char* msg){
 // Set the Prefs error message, overwriting any previous message politely
	if (prefs->errorMsg != NULL){
		free(prefs->errorMsg);
	}
	if (msg != NULL){
		prefs->errorMsg = strdup(msg);
	}
}

static int setMonitorType(struct Prefs *prefs, char* monitorType){
 // The user has specified how they want the monitoring data to be displayed

	int status = FAIL;

	if (strcmp(monitorType, ARG_MONITOR_TYPE_NUMS) == 0) {
		prefs->monitorType = PREF_MONITOR_TYPE_NUMS;
		status = SUCCESS;

	} else if (strcmp(monitorType, ARG_MONITOR_TYPE_BAR) == 0) {
		prefs->monitorType = PREF_MONITOR_TYPE_BAR;
		status = SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_MONITOR_TYPE);
	}

	return status;
}

static int setDirection(struct Prefs *prefs, char* dirTxt){
 // The user has specified the traffic direction (upload/download) they they want to monitor

	int status = FAIL;

	if (strcmp(dirTxt, ARG_DIRECTION_DL) == 0) {
		prefs->direction = PREF_DIRECTION_DL;
		status = SUCCESS;

	} else if (strcmp(dirTxt, ARG_DIRECTION_UL) == 0) {
		prefs->direction = PREF_DIRECTION_UL;
		status = SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_DIRECTION);
	}

	return status;
}

static int setBarChars(struct Prefs *prefs, char* barCharsTxt){
 // The user has specified the maximum number of characters to use when drawing a bar in monitor mode.

	int barChars = strToInt(barCharsTxt, 0);

	if (barChars > 0){
		prefs->barChars = barChars;
		return SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_WIDTH);
		return FAIL;
	}
}

static int setMaxAmount(struct Prefs *prefs, char* maxAmountTxt){
 /* The user has specified the number of bytes that must be transferred for the bar to be drawn
    at maximum length when monitoring - effectively the scale of the graph. */

	int maxAmount = strToInt(maxAmountTxt, 0);

	if (maxAmount > 0){
		prefs->maxAmount = maxAmount;
		return SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_MAX);
		return FAIL;
	}
}

static int setGroup(struct Prefs *prefs, char* groupTxt){
 // The user has specified how they want the results of a query to be grouped

	int status = FAIL;

	if (strcmp(groupTxt, ARG_GROUP_HOURS) == 0) {
		prefs->group = PREF_GROUP_HOURS;
		status = SUCCESS;

	} else if (strcmp(groupTxt, ARG_GROUP_DAYS) == 0) {
		prefs->group = PREF_GROUP_DAYS;
		status = SUCCESS;

	} else if (strcmp(groupTxt, ARG_GROUP_MONTHS) == 0) {
		prefs->group = PREF_GROUP_MONTHS;
		status = SUCCESS;

	} else if (strcmp(groupTxt, ARG_GROUP_YEARS) == 0) {
		prefs->group = PREF_GROUP_YEARS;
		status = SUCCESS;

	} else if (strcmp(groupTxt, ARG_GROUP_TOTAL) == 0) {
		prefs->group = PREF_GROUP_TOTAL;
		status = SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_GROUP);
	}

	return status;
}

static int setRange(struct Prefs *prefs, char* rangeTxt){
 // The user has specified a date/time range for a database query

	int rangeLen = strlen(rangeTxt);

	if (rangeLen > 21){ // yyyymmddhh-yyyymmddhh
	 // Range was too long to be valid
		setErrMsg(prefs, ERR_OPT_BAD_RANGE);
		return FAIL;

	} else {
		char* hyphen    = strchr(rangeTxt, '-');
		char rangeFrom[11];
		char rangeTo[11];

		if (hyphen == 0){
		 // There was no hyphen in the range, so it should be 10 chars or less in length
			if (rangeLen > 10){
				setErrMsg(prefs, ERR_OPT_BAD_RANGE);
				return FAIL;

			} else {
			 /* A range without a hyphen means that the range should cover the entire period
			    specified, eg -r2009 is equivalent to -r2009-2009, ie a range overing the whole
			    of the year 2009 */
				strncpy(rangeFrom, rangeTxt, 11);
				strncpy(rangeTo,   rangeTxt, 11);
			}

		} else {
		 // There was a hyphen, so split the range into its 'from' and 'to' components
			int hyphenPos = hyphen - rangeTxt;
			strncpy(rangeFrom, rangeTxt, hyphenPos);
			rangeFrom[hyphenPos] = 0;

			strncpy(rangeTo, rangeTxt+hyphenPos+1, 11);
		}

	 // Turn the text into timestamps
		int tsFrom = makeTsFromRange(rangeFrom);
		if (tsFrom == INVALID_TS){
			setErrMsg(prefs, ERR_OPT_BAD_RANGE);
			return FAIL;
		}

		int tsTo = makeTsFromRange(rangeTo);
		if (tsTo == INVALID_TS){
			setErrMsg(prefs, ERR_OPT_BAD_RANGE);
			return FAIL;
		}

	 /* We allow the user to mix up the order of the 'from' and 'to' parts of the range,
	    this is where we work out which is which. The timestamp that represents the end
	    of the range needs to be adjusted so that it really marks the end of the
	    interval (eg the range -r2009-2009 needs to cover everything from start of
	    01/01/2009 to end of 31/12/2009) */
		if (tsFrom > tsTo){
			prefs->rangeFrom = tsTo;
			prefs->rangeTo   = adjustForEndOfRange(tsFrom, strlen(rangeFrom));
		} else {
			prefs->rangeFrom = tsFrom;
			prefs->rangeTo   = adjustForEndOfRange(tsTo, strlen(rangeTo));
		}
	}
	return SUCCESS;
}

static time_t makeTsFromRange(char* rangePart){
 // Convert one part of the range argument (ie the 'from' or 'to' part) into a timestamp

 /* There are 4 possible formats that the rangePart value might have:
 		yyyymmddhh - Year, Month, Day, Hour
 		yyyymmdd   - Year, Month, Day
 		yyyymm     - Year, Month
 		yyyy       - Year
 	check the length of the string and if it matches a value format,
 	try to convert it*/

	int rangeLen = strlen(rangePart);
    char fullDate[14];
    strncpy(fullDate,"0000/00/00/00", 14);

	switch(rangeLen){
		case 10: // yyyymmddhh
			strncpy(fullDate + 11, rangePart + 8, 2);
		 	/* FALLTHRU */

		case 8:	// yyyymmdd
			strncpy(fullDate + 8,  rangePart + 6, 2);
		 	/* FALLTHRU */

		case 6: // yyyymm
			strncpy(fullDate + 5,  rangePart + 4, 2);
		 	/* FALLTHRU */

		case 4:	// yyyy
			strncpy(fullDate,      rangePart,     4);
			break;

		default:
			return INVALID_TS;
	}

	#ifdef _WIN32
		struct tm cal = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	#else
		struct tm cal = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	#endif
	cal.tm_isdst = -1; // Get the library to handle Daylight Savings Time
	cal.tm_mday  = 1;  // Default to the first day of the month
	
    strptime(fullDate, "%Y/%m/%d/%H", &cal);

	return mktime(&cal); // Need the GMT/UTC value
}

static time_t adjustForEndOfRange(time_t time, int rangeLen){
 /* The timestamp that was calculated from the end of the range that the user requested
    needs to be adjusted so that it marks the end of the specified time period rather than
    the beginning. */

    struct tm* cal = localtime(&time);
    cal->tm_isdst = -1;

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
			assert(FALSE); // Should have caught invalid range formats already
			break;
	}

	return mktime(cal);
}

static int setHelp(struct Prefs *prefs){
 // The user has requested the help text.
	prefs->help = 1;
	return SUCCESS;
}

static int setVersion(struct Prefs *prefs){
// The user has requested the app version.
	prefs->version = 1;
	return SUCCESS;
}

static int setUnits(struct Prefs *prefs, char* units){
 // The user has specified the units to use when displaying the results

	int status = FAIL;

	if (strcmp(units, ARG_UNITS_BYTES) == 0) {
	 // Show amounts in bytes
		prefs->units = PREF_UNITS_BYTES;
		status = SUCCESS;

	} else if (strcmp(units, ARG_UNITS_ABBREV) == 0) {
	 // Show amounts with abbreviated unit names
		prefs->units = PREF_UNITS_ABBREV;
		status = SUCCESS;

	} else if (strcmp(units, ARG_UNITS_FULL) == 0) {
	 // Show amounts with full unit names
		prefs->units = PREF_UNITS_FULL;
		status = SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_UNIT);
	}

	return status;
}

static int setDumpFormat(struct Prefs *prefs, char* dumpFormat){
 // The user has specified the format to be used when dumping the database contents

	int status = FAIL;

	if (strcmp(dumpFormat, ARG_DUMP_FORMAT_CSV_SHORT) == 0 || strcmp(dumpFormat, ARG_DUMP_FORMAT_CSV_LONG) == 0){
		prefs->dumpFormat = PREF_DUMP_FORMAT_CSV;
		status = SUCCESS;

	} else if (strcmp(dumpFormat, ARG_DUMP_FORMAT_FIXED_WIDTH_SHORT) == 0 || strcmp(dumpFormat, ARG_DUMP_FORMAT_FIXED_WIDTH_LONG) == 0){
		prefs->dumpFormat = PREF_DUMP_FORMAT_FIXED_WIDTH;
		status = SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_DUMP_FORMAT);
	}

	return status;
}

static int setMode(struct Prefs *prefs, char* mode){
 // The user has specified the mode in which they wish to use the application

	int status = FAIL;

	if (strcmp(mode, ARG_MODE_DUMP_SHORT) == 0 || strcmp(mode, ARG_MODE_DUMP_LONG) == 0){
		prefs->mode = PREF_MODE_DUMP;
		status = SUCCESS;

	} else if (strcmp(mode, ARG_MODE_SUMMARY_SHORT) == 0 || strcmp(mode, ARG_MODE_SUMMARY_LONG) == 0){
		prefs->mode = PREF_MODE_SUMMARY;
		status = SUCCESS;

	} else if (strcmp(mode, ARG_MODE_MONITOR_SHORT) == 0 || strcmp(mode, ARG_MODE_MONITOR_LONG) == 0){
		prefs->mode = PREF_MODE_MONITOR;
		status = SUCCESS;

	} else if (strcmp(mode, ARG_MODE_QUERY_SHORT) == 0 || strcmp(mode, ARG_MODE_QUERY_LONG) == 0){
		prefs->mode = PREF_MODE_QUERY;
		status = SUCCESS;

	} else {
		setErrMsg(prefs, ERR_OPT_BAD_MODE);
	}

	return status;
}
