#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"

/*
Contains the code that handles database query requests made via the bmclient utility.
*/

extern struct Prefs prefs;
static void printGroupYears(struct Data* result);
static void printGroupMonths(struct Data* result);
static void printGroupDays(struct Data* result);
static void printGroupHours(struct Data* result);
static void printResultsForInterval(struct Data* result, char* rangeTxt);

static void setDefaultPrefs(){
 // Use these defaults if nothing else is specified by the user
	if (prefs.group == PREF_NOT_SET){
		prefs.group = PREF_GROUP_TOTAL;
	}
	if (prefs.units == PREF_NOT_SET){
		prefs.units = PREF_UNITS_ABBREV;
	}
}

void doBmClientQuery(){
	setDefaultPrefs();

	if (prefs.rangeFrom == 0 && prefs.rangeTo == 0){
	 // Can't do a query without a range...
		printf("No range has been specified. Use '-r' to specify a range, use '-h' to display help.\n");

	} else if (prefs.filter == NULL) {
	 // Need a valid filter name
	 	printf("No filter name has been specfied.\n"); //TODO put proper msg in #define

	} else {
		struct Filter* filters = readFilters();
		struct Filter* filter = getFilterFromName(filters, prefs.filter, NULL);
		
		if (filter == NULL){
			printf("No filter named '%s' could be found.\n", prefs.filter); //TODO put proper msg in #define
			
		} else {
		 // First, print out the date/time range covered by the query
			char date1[11];
			char time1[9];
			char date2[11];
			char time2[9];
	
			toDate(date1, prefs.rangeFrom);
			toTime(time1, prefs.rangeFrom);
			toDate(date2, prefs.rangeTo);
			toTime(time2, prefs.rangeTo);
	
			printf("From: %s %s\n", date1, time1);
			printf("To:   %s %s\n", date2, time2);
	
		 // Run the query here
	        struct Data* data = getQueryValues(prefs.rangeFrom, prefs.rangeTo, prefs.group, filter->id);
	        struct Data* initData = data;
	
		 // How we display the results depends on how they are grouped...
			switch(prefs.group){
				case PREF_GROUP_HOURS:
					printGroupHours(data);
					break;
	
				case PREF_GROUP_DAYS:
					printGroupDays(data);
					break;
	
				case PREF_GROUP_MONTHS:
					printGroupMonths(data);
					break;
	
				case PREF_GROUP_YEARS:
					printGroupYears(data);
					break;
	
				case PREF_GROUP_TOTAL:
					printResultsForInterval(data, "Total:");
					break;
	
				default:
					assert(FALSE); // Should have caught invalid/missing group values already
					break;
			}
	
			freeData(initData);
		}
		freeFilters(filters);
	}
}

static char* makeYearTxt(int ts, char* txt){
 // Write the 4-digit year value for this timestamp
	strftime(txt, 5, "%Y", localtime((time_t *) &ts));
	return txt;
}

static void printGroupYears(struct Data* result){
	char yearTxt[5];

	while(result != NULL){
	 /* Print each item, preceeded by the year that it represents. Subtract the 'dr' (duration)
	    from the timestamp before output so that we are displaying the correct date value for the
	    start of the interval. */
		printResultsForInterval(result, makeYearTxt(result->ts - result->dr, yearTxt));
        result = result->next;
	}
}

static char* makeMonthTxt(int ts, char* txt){
 // Write the 3-letter month, and 4-digit year values for this timestamp
	strftime(txt, 9, "%b %Y", localtime((time_t *) &ts));
	return txt;
}

static void printGroupMonths(struct Data* result){
	char monthTxt[9];

	while(result != NULL){
	 /* Print each item, preceeded by the month and year that it represents. Subtract the 'dr' (duration)
	    from the timestamp before output so that we are displaying the correct date value for the
	    start of the interval. */
		printResultsForInterval(result, makeMonthTxt(result->ts - result->dr, monthTxt));
        result = result->next;
	}
}

static char* makeDayTxt(int ts, char* txt){
 // Write the 2-digit day, 3-letter month, and 4-digit year values for this timestamp
	strftime(txt, 12, "%d %b %Y", localtime((time_t *) &ts));
	return txt;
}

static void printGroupDays(struct Data* result){
	char dayTxt[12];

	while(result != NULL){
	 /* Print each item, preceeded by the day, month and year that it represents. Subtract
	 	the 'dr' (duration) from the timestamp before output so that we are displaying the
	 	correct date value for the start of the interval. */
		printResultsForInterval(result, makeDayTxt(result->ts - result->dr, dayTxt));
        result = result->next;
	}
}

static char* makeHourTxt(int ts, char* txt){
 // Write the 24-hour clock hour value for this timestamp
	strftime(txt, 6, "%H:00", localtime((time_t *) &ts));
	return txt;
}

static void printGroupHours(struct Data* result){
	char dayTxt[12];
	char time1Txt[6];
	char time2Txt[6];
	char hourTxt[23];

	time_t ts;

	while(result != NULL){
	 /* Print each item, preceeded by the time interval, and date, that it represents.
	 	Subtract the 'dr' (duration) from the timestamp before output so that we are
	 	displaying the correct date/time value for the start of the interval. */
	 	ts = result->ts - result->dr;

		sprintf(hourTxt, "%s %s-%s", makeDayTxt(ts, dayTxt), makeHourTxt(ts, time1Txt), makeHourTxt(result->ts, time2Txt));
		printResultsForInterval(result, hourTxt);
		result = result->next;
	}
}

static void printResultsForInterval(struct Data* result, char* rangeTxt){
 // Print the date/time range for this interval, followed by the formatted ul/dl values
	char vlTxt[20];

	if (result != NULL){
        formatAmount(result->vl, TRUE, (prefs.units == UNITS_ABBREV), vlTxt);
	} else {
	    formatAmount(0, TRUE, (prefs.units == UNITS_ABBREV), vlTxt);
	}
	printf("%s %s\n", rangeTxt, vlTxt);
}
