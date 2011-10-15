#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"

/*
Contains the code that handles monitor requests made via the bmclient utility.
*/

static void sigIntHandler();
static int stopNow = FALSE;
extern struct Prefs prefs;
static void printBar(BW_INT vl);
static void printText(BW_INT vl);

static void setDefaultPrefs(){
 // Use these defaults if nothing else is specified by the user
	if (prefs.monitorType == PREF_NOT_SET){
		prefs.monitorType = PREF_MONITOR_TYPE_NUMS;
	}
	if (prefs.barChars == PREF_NOT_SET){
		prefs.barChars = DEFAULT_BAR_CHARS;
	}
	if (prefs.maxAmount == PREF_NOT_SET){
		prefs.maxAmount = DEFAULT_MAX_AMOUNT;
	}
}

void doMonitor(){
	struct Filter* filters = readFilters();
	struct Filter* filter;
	
	if ((filter = getFilterFromName(filters, prefs.filter, NULL)) == NULL) {
		if (prefs.filter == NULL){
			printf("No filter name has been specified, use -n <filter name>, or use '-h' to display help.\n");
		} else {
			printf("No filter named '%s' was found, used 'bmdb showfilters' to display the list.");	
		}
		
	} else {
		setDefaultPrefs();
	
	 // We are going to loop forever unless we get an interrupt
		signal(SIGINT, sigIntHandler);
	
	 // Check how often new values get written to the database - this is how often we update the display
		int dbWriteInterval = getConfigInt(CONFIG_DB_WRITE_INTERVAL, TRUE);
		if (dbWriteInterval < 1){
			dbWriteInterval = 1;	
		}
	
		if (dbWriteInterval == 1){
			printf("Monitoring '%s'... (Ctrl-C to abort)\n", filter->desc);
		} else {
			printf("Monitoring '%s' [updating every %d secs]... (Ctrl-C to abort)\n", filter->desc, dbWriteInterval);	
		}
		struct Data* values;
		struct Data* currentValue;
		int doBars = (prefs.monitorType == PREF_MONITOR_TYPE_BAR);
		BW_INT vl;
		
		while(!stopNow){
			vl = 0;
	
		 // Get the values for the appropriate time-span
		 	time_t now = getTime();
			values = getMonitorValues(now - dbWriteInterval, filter->id);
	
			if (values == NULL){
			 // We print out zeroes if there is nothing from the db
	            values = allocData();
			}
	        currentValue = values;
	
			while(currentValue != NULL){
			 // If system clock gets put back we don't want to include all the values that now lie in the future
				if (currentValue->ts <= now){
					vl += currentValue->vl;
				}
				currentValue = currentValue->next;
			}
	
			if (doBars){
			 // We need to 'draw' a bar to represent the data
				printBar(vl);
			} else {
			 // We just need to display the figures
				printText(vl);
			}
	
			freeData(values);
		    doSleep(dbWriteInterval);
		}
		printf("monitoring aborted.\n");
	}
	freeFilters(filters);
}

static void printBar(BW_INT vl){
 /* We must draw a bar to represent either the upload/download speed, as well as
 	displaying a numeric value. */

 // This is where we put the numeric part
	char vlTxt[20];
	formatAmount(vl, TRUE, TRUE, vlTxt);

 // Work out how many characters this bar will occupy
	int charCount = prefs.barChars * ((float)vl / prefs.maxAmount);
	charCount = (charCount > prefs.barChars) ? prefs.barChars : charCount;

 // Display the numeric value
	printf("%10s|", vlTxt);

 // Draw the bar
	int i;
	for(i=1; i<= charCount; i++){
		printf("#");
	}
	printf("\n");
}

static void printText(BW_INT vl){
	printf("%llu\n", vl);
}

static void sigIntHandler(){
	stopNow = TRUE;
}
