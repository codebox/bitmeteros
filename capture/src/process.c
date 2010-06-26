/*
 * BitMeterOS
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
 */

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

struct Data* extractDiffs(struct Data*, struct Data*);
static int tsCompress;

void setupCapture(){
 // Called once when the application starts - setup up the various db related things...
    openDb();
    prepareDb();
    dbVersionCheck();
	setupDb();
	compressDb();

	prevData = getData();
	tsCompress = getNextCompressTime();
}

int processCapture(){
    int status;

 // Called continuously by the main processing loop of the application
	int ts = getTime();

 // Get the current values for each network adapter
	struct Data* currData = getData();

 // Calculate the differences between the current values and the previous ones
	struct Data* diffList = extractDiffs(prevData, currData);

 // Write the changes in values to the database
	status = updateDb(ts, POLL_INTERVAL, diffList);

	logData(diffList);
	freeData(diffList);
	freeData(prevData);

	if (status == FAIL){
        return FAIL;
	}

 // Save the current values so we can compare against them next time
	prevData = currData;

 // Is it time to compress the database yet?
	if (ts > tsCompress){
		status = compressDb();
		if (status == FAIL){
            return FAIL;
		}
		tsCompress = getNextCompressTime();
	}

	return status;
}

void shutdownCapture(){
 // Called when the application shuts down
	closeDb();
}


struct Data* extractDiffs(struct Data* oldList, struct Data* newList){
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
	while(data != NULL){
		logMsg(LOG_INFO, "%d DL=%lu UL=%lu $s", getTime(), data->dl, data->ul, data->ad);
		data = data->next;
	}
}

