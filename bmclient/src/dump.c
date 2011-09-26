#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"

/*
Contains the code that handles database dump requests made via the bmclient utility.
*/

extern struct Prefs prefs;
static int valueColWidth = 0;
static struct Filter* filters;

static struct DumpCalls calls = {&calcMaxValue, &readFilters, &toTime,
		&toDate, &formatAmountByUnits, &getFilterFromId};

static struct DumpCalls getCalls(){
	#ifdef UNIT_TESTING
		return mockDumpCalls;
	#else
		return calls;
	#endif
}

static void setDefaultPrefs(){
 // Use these defaults if nothing else is specified by the user
	if (prefs.units == PREF_NOT_SET){
		prefs.units = PREF_UNITS_BYTES;
	}
	if (prefs.dumpFormat == PREF_NOT_SET){
		prefs.dumpFormat = PREF_DUMP_FORMAT_FIXED_WIDTH;
	}
}

static void printRow(int ignored, struct Data* row){
 /* We want to print out the date of the beginning of each interval in the dump, so
 	compute this by subtracting the duration (dr) from the timestamp (ts). */
	char date[11];
	getCalls().toDate(date, row->ts - row->dr);

 // This is the time marking the start of the interval
	char timeFrom[9];
	getCalls().toTime(timeFrom, row->ts - row->dr);

 // This is the time marking the end of the interval
	char timeTo[9];
	getCalls().toTime(timeTo, row->ts);

 // Format the values
	char vlTxt[20];
	getCalls().formatAmountByUnits(row->vl, vlTxt, prefs.units);

	struct Filter* filter = getCalls().getFilterFromId(filters, row->fl);

	if (prefs.dumpFormat == PREF_DUMP_FORMAT_CSV){
	 // Output in CSV is easy, just print it
		printf("%s,%s,%s,%d,%s,%s\n", date, timeFrom, timeTo, row->dr, vlTxt, filter->name);

	} else if (prefs.dumpFormat == PREF_DUMP_FORMAT_FIXED_WIDTH){
	 // For fixed-width output we need to work out how wide to make the columns
		int vlWidth;
		switch (prefs.units) {
			case PREF_UNITS_BYTES:
			 // We should have retrieved the max ul/dl values earlier
			 	#ifndef UNIT_TESTING
					assert(valueColWidth != 0);
				#endif

			 // From the max values we know how wide to make the columns
				vlWidth = valueColWidth;
				break;

			case PREF_UNITS_ABBREV:
				#ifndef UNIT_TESTING
					assert(valueColWidth == 0);
				#endif
			 // Abbreviated units means that 10 chars will always be enough: '1023.99 Gb'
				vlWidth = 10;
				break;

			case PREF_UNITS_FULL:
				#ifndef UNIT_TESTING
					assert(valueColWidth == 0);
				#endif
			 // Full units means that 17 chars will always be enough: '1023.99 Gigabytes'
				vlWidth = 17;
				break;

			default:
				assert(FALSE); // Should have caught invalid units before now
				break;
		}

		printf("%s %s %s %4d %*s %s\n", date, timeFrom, timeTo, row->dr, vlWidth, vlTxt, filter->name);

	}
	freeData(row);
}

void doDump(){
	setDefaultPrefs();

 // We need to retrieve details of the filters so we can print the correct filter name for each row
	filters = getCalls().readFilters();

 /* If we are displaying the ul/dl values in bytes, and we are using a fixed width output
 	format, then we need to calculate the larges ul/dl values so we know how wide to make
 	each column */
	int calcColWidths = (prefs.units == PREF_UNITS_BYTES) && (prefs.dumpFormat == PREF_DUMP_FORMAT_FIXED_WIDTH);
	if (calcColWidths){
	 // We need a transaction because the max values must be consistent with the data that we are dumping out
        beginTrans(FALSE);
        struct Data* data = getCalls().calcMaxValue();
		valueColWidth = (int) log10(data->vl) + 1;
		freeData(data);
	}

 // Invoke printRow once for each db row
	getDumpValues(0, &printRow);

	if (calcColWidths){
	 // Clean up
		commitTrans();
	}
	
	freeFilters(filters);
}
