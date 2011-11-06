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
    signal(SIGINT,  sigHandler);    // Trap Ctrl-C
    signal(SIGTERM, sigHandler);    // Trap termination requests from the system

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
