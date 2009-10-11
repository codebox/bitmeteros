#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"

static void printRow(struct Data* row);
extern struct Prefs prefs;
static struct Data* maxValues = NULL;

static void setDefaultPrefs(){
	if (prefs.units == PREF_NOT_SET){
		prefs.units = PREF_UNITS_BYTES;
	}
	if (prefs.dumpFormat == PREF_NOT_SET){
		prefs.dumpFormat = PREF_DUMP_FORMAT_FIXED_WIDTH;
	}
}
void doDump(){
	setDefaultPrefs();
	
 /* If we are displaying the ul/dl values in bytes, and we are using a fixed width output 
 	format, then we need to calculate the larges ul/dl values so we know how wide to make 
 	each column */
	int computeMaxValues = (prefs.units == PREF_UNITS_BYTES) && (prefs.dumpFormat == PREF_DUMP_FORMAT_FIXED_WIDTH);

	if (computeMaxValues){
	 // We need a transaction because the max values must be consistent with the data that we are dumping out
        beginTrans();
		maxValues = calcMaxValues();
	}

 // Invoke printRow once for each db row 
	getDumpValues(&printRow);

	if (computeMaxValues){
	 // Clean up
		commitTrans();
		freeData(maxValues);
	}
}

static void printRow(struct Data* row){
 /* We want to print out the date of the beginning of each interval in the dump, so 
 	compute this by subtracting the duration (dr) from the timestamp (ts). */
	char date[11];
	toDate(date, row->ts - row->dr);

 // This is the time marking the start of the interval
	char timeFrom[9];
	toTime(timeFrom, row->ts - row->dr);
	
 // This is the time marking the end of the interval
	char timeTo[9];
	toTime(timeTo, row->ts);

 // Format the ul/dl values
	char* dlTxt = (char*) calloc(20, sizeof(char));
	char* ulTxt = (char*) calloc(20, sizeof(char));
	formatAmounts(row->dl, row->ul, dlTxt, ulTxt, prefs.units);

	if (prefs.dumpFormat == PREF_DUMP_FORMAT_CSV){
	 // Output in CSV is easy, just print it
		printf("%s,%s,%s,%d,%s,%s\n", date, timeFrom, timeTo, row->dr, dlTxt, ulTxt);

	} else if (prefs.dumpFormat == PREF_DUMP_FORMAT_FIXED_WIDTH){
	 // For fixed-width output we need to work out how wide to make the columns
		int dlWidth, ulWidth;
		switch (prefs.units) {
			case PREF_UNITS_BYTES:
			 // We should have retrieved the max ul/dl values earlier
				assert(maxValues != NULL);
				
			 // From the max values we know how wide to make the columns
				dlWidth = 1 + (int) log10(maxValues->dl);
				ulWidth = 1 + (int) log10(maxValues->ul);
				break;

			case PREF_UNITS_ABBREV:
				assert(maxValues == NULL);
			 // Abbreviated units means that 10 chars will always be enough: '1023.99 Gb'
				dlWidth = ulWidth = 10;
				break;

			case PREF_UNITS_FULL:
				assert(maxValues == NULL);
			 // Full units means that 17 chars will always be enough: '1023.99 Gigabytes'
				dlWidth = ulWidth = 17;
				break;

			default:
				//TODO
				break;
		}
		printf("%s %s %s %4d %*s %*s\n", date, timeFrom, timeTo, row->dr, dlWidth, dlTxt, ulWidth, ulTxt);

	}
	freeData(row);
}
