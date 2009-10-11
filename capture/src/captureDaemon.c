#include <signal.h>
#include "capture.h"
#include "common.h"

/* Entry point for the application. */

static int stopNow = 0;
static void sigHandler();

int main(int argc, char **argv){
 // Initialise logging and database
	setLogLevel(LOG_ERR);
	setLogToFile(1);
	setupCapture();

 // We stop when either of these happen
	signal(SIGINT,  sigHandler);	// Trap Ctrl-C
	signal(SIGTERM, sigHandler);	// Trap termination requests from the system

 // Loop until one of the signal handlers is triggered
    int status;
	while(!stopNow){
    	status = processCapture();
    	if (status == FAIL){
            stopNow = TRUE;
    	}
	}

	shutdownCapture();
	return 0;
}

static void sigHandler(){
	stopNow = 1;
}
