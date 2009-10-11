#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "bmclient.h"

/*
	Modes:
		dump    - extracts all db data
		summary - prints summary of data
		monitor - updates display at intervals, draw bars?
		query   -
	Opts:
	  - flag for 1000/1024 (SI/binary) bytes per kilobyte
*/

struct Prefs prefs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};

int main(int argc, char **argv){
	int argsOk = parseArgs(argc, argv, &prefs);

	if (!argsOk){
		if (prefs.errorMsg != NULL){
			printf("Error: %s\n", prefs.errorMsg);
		} else {
			printf("BitMeter did not understand. ");
		}
		printf("Use the '-h' option to display help.\n");

	} else if (prefs.help){
		doHelp();

	} else if (prefs.version){
		doVersion();

	} else {
		openDb();

		switch(prefs.mode){
			case PREF_MODE_DUMP:
				doDump();
				break;
			case PREF_MODE_SUMMARY:
				doSummary();
				break;
			case PREF_MODE_MONITOR:
				doMonitor();
				break;
			case PREF_MODE_QUERY:
				doQuery();
				break;
			default:
				printf("???\n");
				//TODO
				break;
		}

		closeDb();
	}

	if (prefs.errorMsg != NULL){
		free(prefs.errorMsg);
	}

	return 0;
}
