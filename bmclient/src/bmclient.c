#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "bmclient.h"

/*
Contains the entry-point for the bmclient command-line utility.
*/

// This struct get populated by the various flags/options read from the command-line
struct Prefs prefs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};

static struct BmClientCalls calls = {&doHelp, &doVersion, &doDump, &doMonitor, 
		&doSummary, &doQuery, &setLogLevel, &parseArgs, &openDb, &closeDb, &dbVersionCheck};	

static struct BmClientCalls getCalls(){
	#ifdef UNIT_TESTING	
		return mockBmClientCalls;
	#else
		return calls;
	#endif
}

#ifndef UNIT_TESTING	
	int main(int argc, char **argv){
		return _main(argc, argv);	
	}
#endif

int _main(int argc, char **argv){
 // Interpret the command-lin arguments and decide if they make sense
	int status = getCalls().parseArgs(argc, argv, &prefs);

    printf(COPYRIGHT);
	getCalls().setLogLevel(LOG_INFO);
	
	if (status == FAIL){
	 // The command-line was duff...
		if (prefs.errorMsg != NULL){
		 // ...and we have a specific error to show the user
			printf("Error: %s\n", prefs.errorMsg);
		} else {
		 // ...and we have no specific error message, so show a vague one
			printf(ERR_WTF);
		}
		printf(SHOW_HELP);

	} else if (prefs.help){
	 // Dump the help info and stop
		getCalls().doHelp();

	} else if (prefs.version){
	 // Show the version and stop
		getCalls().doVersion();

	} else {
	 // We will need to go to the database if we end up here
		getCalls().openDb();
        getCalls().dbVersionCheck();

		switch(prefs.mode){
			case PREF_MODE_DUMP:
				getCalls().doDump();
				break;
			case PREF_MODE_SUMMARY:
				getCalls().doSummary();
				break;
			case PREF_MODE_MONITOR:
				getCalls().doMonitor();
				break;
			case PREF_MODE_QUERY:
				getCalls().doQuery();
				break;
			default:
				assert(FALSE); // Any other mode value should cause parseArgs to fail
				break;
		}

		getCalls().closeDb();
	}

	return 0;
}
