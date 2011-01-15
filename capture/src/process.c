/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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
 */

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include "capture.h"
#include "common.h"
#include <string.h>

/*
Contains the high-level code that is invoked by the main processing loop of the application. The code
in here co-ordinates the various data capture and storage invocations, as well as performing the required
initialisation, and termination steps.
*/

/* Holds the data captured during the previous invocation of processCapture, need to hold on to this so we can compare
   current values with previous ones and thereby calculate the differences. */
static struct Data* prevData;

struct Data* extractDiffs(int ts, struct Data*, struct Data*);
static int tsCompress;
static int unwrittenDiffCount = 0;
static int dbWriteInterval;
static struct Data* unwrittenDiffs = NULL;

static void setCustomLogLevel(){
 // If a custom logging level for the capture process has been set in the db then use it
	int dbLogLevel = getConfigInt(CONFIG_CAP_LOG_LEVEL, TRUE);
	if (dbLogLevel > 0) {
		setLogLevel(dbLogLevel);
	}
}

#ifdef TESTING
void setPrevData(struct Data* data){
	prevData = data;	
}
void setDbWriteInterval(int interval){
	dbWriteInterval = interval;
}
#endif

void setupCapture(){
 // Called once when the application starts - setup up the various db related things...
    openDb();
    setCustomLogLevel();
    dbVersionCheck();
	setupDb();
	compressDb();

	prevData = getData();
	tsCompress = getNextCompressTime();
	
 // Check how often we should write captured values to the database - the default is every second
	dbWriteInterval = getConfigInt(CONFIG_DB_WRITE_INTERVAL, TRUE);
	if (dbWriteInterval < 1){
		dbWriteInterval = 1;	
	}
}
static int writeUnwrittenDiffs(){
	int status = updateDb(POLL_INTERVAL, unwrittenDiffs);
	
	logData(unwrittenDiffs);
	freeData(unwrittenDiffs);
	unwrittenDiffs = NULL;	
	
	return status;
}

int processCapture(){
    int status;

 // Called continuously by the main processing loop of the application
	int ts = getTime();

 // Get the current values for each network adapter
	struct Data* currData = getData();

 // Calculate the differences between the current values and the previous ones
	struct Data* diffList = extractDiffs(ts, prevData, currData);
	appendData(&unwrittenDiffs, diffList);
	
 // Save the current values so we can compare against them next time
	freeData(prevData);
	prevData = currData;

 // Is it time to write the values to the database yet?
	if (++unwrittenDiffCount >= dbWriteInterval) {
	 // Write the changes in values to the database
		status = writeUnwrittenDiffs();
	
		if (status == FAIL) {
	        return FAIL;
		}
	
	 // Is it time to compress the database yet?
		if (ts > tsCompress) {
			status = compressDb();
			if (status == FAIL){
	            return FAIL;
			}
			tsCompress = getNextCompressTime();
		}
		
		unwrittenDiffCount = 0;	
	} else {
		status = SUCCESS;	
	}

	return status;
}

void shutdownCapture(){
 // Called when the application shuts down
 	writeUnwrittenDiffs();
		
	closeDb();
}


struct Data* extractDiffs(int ts, struct Data* oldList, struct Data* newList){
 /* Accepts 2 lists of Data structs and iterates through them looking for adapter names that
    match. When a matching pair is found the differences between the dl and ul figures are calculated
    and if they are non-zero the delta is stored in the 'diffData' list which is evetually returned. This
    routine must handle cases where one or both lists are null, where the adapters do not appear in the
    same order within the 2 lists, and where adapters are present in one list and not in the other (this
    will happen if adapters are enabled or disabled while the application is running). */
	BW_INT dl,ul;

 // These pointers reference the Data structs that we are currently examining, from each of the 2 lists
	struct Data* oldData;
	struct Data* newData;

	struct Data* diffData = NULL;
	struct Data* newDiff = NULL;

	oldData = oldList; // Point oldData at the start of the oldList
	while(oldData != NULL){
		newData = newList;  // Point newData at the start of the newList
		while(newData != NULL){
			if (strcmp(oldData->ad, newData->ad) == 0){
             // We found an adapter that appears in both lists, so look at the dl and ul values
			    if ((newData->dl >= oldData->dl) && (newData->ul >= oldData->ul)){;
                    dl = newData->dl - oldData->dl;
                    ul = newData->ul - oldData->ul;
                    if (dl > 0 || ul > 0){
                     // At least 1 of the values increased since last time, so we add an item the the diffData list for this adapter
                        newDiff = allocData();
                        newDiff->dl = dl;
                        newDiff->ul = ul;
                        newDiff->ts = ts;
                        setAddress(newDiff, oldData->ad);
                        setHost(newDiff, oldData->hs);

                        appendData(&diffData, newDiff);
                    }
			    } else {
                    logMsg(LOG_WARN, "Values wrapped around for adapter %s", oldData->ad);
			    }
			    break; // We found the match so no point looking at any remaining items in newList
			}
			newData = newData->next; // We didn't find a match so move to the next item in the newList
		}
		oldData = oldData->next;
	}

	return diffData;
}

void logData(struct Data* data){
	if (isLogDebug()){
		while(data != NULL){
			logMsg(LOG_DEBUG, "%d DL=%llu UL=%llu %s", getTime(), data->dl, data->ul, data->ad);
			data = data->next;
		}
	}
}

