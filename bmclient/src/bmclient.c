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

#ifndef UNIT_TESTING	
	int main(int argc, char **argv){
		return _main(argc, argv);	
	}
#endif

int _main(int argc, char **argv){
 // Interpret the command-lin arguments and decide if they make sense
	int status = PARSE_ARGS(argc, argv, &prefs);

    printf(COPYRIGHT);
	SET_LOG_LEVEL(LOG_INFO);
	
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
		DO_HELP();

	} else if (prefs.version){
	 // Show the version and stop
		DO_VERSION();

	} else {
	 // We will need to go to the database if we end up here
		OPEN_DB();
        DB_VERSION_CHECK();

		switch(prefs.mode){
			case PREF_MODE_DUMP:
				DO_DUMP();
				break;
			case PREF_MODE_SUMMARY:
				DO_SUMMARY();
				break;
			case PREF_MODE_MONITOR:
				DO_MONITOR();
				break;
			case PREF_MODE_QUERY:
				DO_QUERY();
				break;
			default:
				assert(FALSE); // Any other mode value should cause parseArgs to fail
				break;
		}

		CLOSE_DB();
	}

	return 0;
}
