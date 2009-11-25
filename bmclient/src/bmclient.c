/*
 * BitMeterOS v0.2.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
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
 *
 * Build Date: Wed, 25 Nov 2009 10:48:23 +0000
 */

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
struct Prefs prefs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};

int main(int argc, char **argv){
 // Interpret the command-lin arguments and decide if they make sense
	int status = parseArgs(argc, argv, &prefs);

    printf(COPYRIGHT);

	if (status == FAIL){
	 // The command-line was duff...
		if (prefs.errorMsg != NULL){
		 // ...and we have a specific error to show the user
			printf("Error: %s\n", prefs.errorMsg);
		} else {
		 // ...and we have no specific error message, so show a vague one
			printf("BitMeter did not understand. ");
		}
		printf("Use the '-h' option to display help.\n");

	} else if (prefs.help){
	 // Dump the help info and stop
		doHelp();

	} else if (prefs.version){
	 // Show the version and stop
		doVersion();

	} else {
	 // We will need to go to the database if we end up here
		openDb();
		prepareDb();
        dbVersionCheck();

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
				assert(FALSE); // Any other mode value should cause parseArgs to fail
				break;
		}

		closeDb();
	}

	return 0;
}
