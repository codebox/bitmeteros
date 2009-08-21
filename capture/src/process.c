#include <stdio.h>
#include <stdlib.h>
#include "capture.h"
#include "common.h"
#include <string.h>

/*  Contains the high-level processing code that is invoked by the main processing loop of the application. The code
    in here co-ordinates the various data capture and storage invocations, as well as performing the required initialisation,
    and termination steps. */

/* Holds the data captured during the previous invocation of processCapture, need to hold on to this so we can compare
   current values with previous ones and thereby calculate the differences. */
static struct BwData* prevData;

struct BwData* extractDiffs(struct BwData*, struct BwData*);
static void freeBwData(struct BwData* data);
static int tsCompress;

void setupCapture(){
 // Called once when the application starts - setup up the various db related things...
    openDb();
	setupDb();
	compressDb();

	prevData = getData();
	tsCompress = getNextCompressTime();
}

void processCapture(){
 // Called continuously by the main processing loop of the application
	doSleep(1);
	int ts = getTime();

 // Get the current values for each network adapter
	struct BwData* currData = getData();

 // Calculate the differences between the current values and the previous ones
	struct BwData* diffList = extractDiffs(prevData, currData);

 // Write the changes in values to the database
	updateDb(ts, POLL_INTERVAL, diffList);
	logBw(diffList);

	freeBwData(prevData);

 // Save the current values so we can compare against them next time
	prevData = currData;

 // Is it time to compress the database yet?
	if (ts > tsCompress){
		compressDb();
		tsCompress = getNextCompressTime();
	}
}

void shutdownCapture(){
 // Called when the application shuts down
	closeDb();
}


static void freeBwData(struct BwData* data){
 // Frees up the memory used by the old BwData values that are about to be discarded
	struct BwData* tmpNext;

	while(data != NULL){
		tmpNext = data->next;
		free(data);
		data = tmpNext;
	}
}

int isSameAddress(struct BwData* data1, struct BwData* data2){
 // Compares 2 network adapter names/addresses TODO cant we just treat them like strings and do strcmp?
	if (data1->addrLen != data2->addrLen){
		return 0;
	} else {
		if (memcmp(&(data1->addr), &(data2->addr), data1->addrLen)==0){
			return 1;
		} else {
			return 0;
		}
	}
}

struct BwData* extractDiffs(struct BwData* oldList, struct BwData* newList){
 /* Accepts 2 lists of BwData structs and iterates through them looking for adapter names that
    match. When a matching pair is found the differences between the dl and ul figures are calculated
    and if they are non-zero the delta is stored in the 'diffData' list which is evetually returned. This
    routine must handle cases where one or both lists are null, where the adapters do not appear in the
    same order within the 2 lists, and where adapters are present in one list and not in the other (this
    will happen if adapters are enabled or disabled while the application is running). */
	unsigned long dl,ul;

 // These pointers reference the BwData structs that we are currently examining, from each of the 2 lists
	struct BwData* oldData;
	struct BwData* newData;

	struct BwData* diffData = NULL;
	struct BwData* lastDiff = NULL;
	struct BwData* newDiff = NULL;

	oldData = oldList; // Point oldData at the start of the oldList
	while(oldData != NULL){
		newData = newList;  // Point newData at the start of the newList
		while(newData != NULL){
			if (isSameAddress(oldData, newData)){
             // We found an adapter that appears in both lists, so look at the dl and ul values
			    if ((newData->dl >= oldData->dl) && (newData->ul >= oldData->ul)){;
                    dl = newData->dl - oldData->dl;
                    ul = newData->ul - oldData->ul;
                    if (dl > 0 || ul > 0){
                     // At least 1 of the values increased since last time, so we add an item the the diffData list for this adapter
                        newDiff = malloc(sizeof(struct BwData));
                        newDiff->dl = dl;
                        newDiff->ul = ul;
                        newDiff->addrLen = oldData->addrLen;
                        memcpy(&(newDiff->addr), &(oldData->addr), oldData->addrLen);
                        newDiff->next = NULL;

                        if (diffData==NULL){
                            diffData = newDiff;
                            lastDiff = newDiff;
                        } else {
                            lastDiff->next = newDiff;
                            lastDiff = newDiff;
                        }
                    }
			    } else {
                    logMsg(LOG_WARN, "Values wrapped around for adapter %s\n", oldData->addr);
			    }
			    break; // We found the match so no point looking at any remaining items in newList
			}
			newData = newData->next; // We didn't find a match so move to the next item in the newList
		}
		oldData = oldData->next;
	}

	return diffData;
}

void logBw(struct BwData* data){
	while(data != NULL){
		logMsg(LOG_INFO, "%d DL=%lu UL=%lu\n", getTime(), data->dl, data->ul);
		data = data->next;
	}
}

