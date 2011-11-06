#include <signal.h>
#include <stdio.h>
#include "common.h"
#include "bmws.h"

static void sigHandler();

int main(){
    setupWeb(); 
    signal(SIGINT, sigHandler); // Trap Ctrl-C
    printf(COPYRIGHT);
    printf("Web Server running, press Ctrl-C to quit..." EOL);
    
    while (TRUE){
        processWeb();
    }

    return 0;
}

static void sigHandler(){
    shutdownWeb();
    printf("Web Server stopped." EOL);
    exit(0);
}
