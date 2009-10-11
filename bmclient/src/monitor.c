#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "common.h"
#include "bmclient.h"
#include "client.h"

static void sigIntHandler();
static int stopNow=0;
extern struct Prefs prefs;
static void printBar(struct Data*);
static void printText(struct Data*);

static void setDefaultPrefs(){
	if (prefs.monitorType == PREF_NOT_SET){
		prefs.monitorType = PREF_MONITOR_TYPE_NUMS;
	}
	if (prefs.direction == PREF_NOT_SET){
		prefs.direction = PREF_DIRECTION_DL;
	}
	if (prefs.barChars == PREF_NOT_SET){
		prefs.barChars = DEFAULT_BAR_CHARS;
	}
	if (prefs.maxAmount == PREF_NOT_SET){
		prefs.maxAmount = DEFAULT_MAX_AMOUNT;
	}
}

void doMonitor(){
	setDefaultPrefs();
	signal(SIGINT, sigIntHandler);

	printf("Monitoring... (Ctrl-C to abort)\n");
	struct Data* values;
	int doBars = (prefs.monitorType == PREF_MONITOR_TYPE_BAR);

	while(!stopNow){
		values = getMonitorValues(getTime()-1);
		if (values == NULL){
		 // We print out zeroes if there is nothing from the db
            values = allocData();
		}

		if (doBars){
			printBar(values);
		} else {
			printText(values);
		}

		freeData(values);
	    doSleep(1);
	}
	printf("monitoring aborted.\n");
}

static void printBar(struct Data* values){
 /* We must draw a bar to represent either the upload/download speed, as well as
 	displaying a numeric value. */

 // This is where we put the numeric part
	char amtTxt[20];
	BW_INT amt = ((prefs.direction == PREF_DIRECTION_DL) ? values->dl : values->ul);
	formatAmount(amt, 1, 1, amtTxt);

 // Work out how many characters this bar will occupy
	int charCount = prefs.barChars * ((float)amt / prefs.maxAmount);
	charCount = (charCount > prefs.barChars) ? prefs.barChars : charCount;

 // Display the numeric value
	printf("%10s|", amtTxt);

 // Draw the bar
	int i;
	for(i=1; i<= charCount; i++){
		printf("#");
	}
	printf("\n");
}
static void printText(struct Data* values){
	printf("DL: %llu UL: %llu\n", values->dl, values->ul);
}

static void sigIntHandler(){
	stopNow = 1;
}
