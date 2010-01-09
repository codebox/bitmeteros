/*
 * BitMeterOS v0.3.0
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
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

#include <signal.h>
#include "capture.h"
#include "common.h"

/*
Contains the entry point for the data capture application, when run as an unmanaged executable.
*/

static int stopNow = FALSE;
static void sigHandler();

int main(int argc, char **argv){
	#ifdef _WIN32
	 // This gets run from the command-line in Windows, so show the Copyright etc
		printf(COPYRIGHT);
		printf("Capturing... (Ctrl-C to abort)\n");
	#endif
	
 // Initialise logging and database
	setLogLevel(LOG_ERR);
	setAppName("CAPTURE");
	setLogToFile(TRUE);
	setupCapture();

 // We stop when either of these happen
	signal(SIGINT,  sigHandler);	// Trap Ctrl-C
	signal(SIGTERM, sigHandler);	// Trap termination requests from the system

 // Loop until one of the signal handlers is triggered
    int status;
	while(!stopNow){
		doSleep(1);
    	status = processCapture();
    	if (status == FAIL){
            stopNow = TRUE;
    	}
	}

	shutdownCapture();
	return 0;
}

static void sigHandler(){
	stopNow = TRUE;
}
