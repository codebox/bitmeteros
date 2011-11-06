#include <stdio.h>
#include <stdlib.h>
#include "bmclient.h"
#include "common.h"

/*
Contains the code that handles help and version requests made via the bmclient utility.
Help text is read from the helpText.c file which is generated during the build process.
*/

extern char* helpTxt;

void doHelp(){
    doBmClientVersion();
    printf(helpTxt);
}

void doBmClientVersion(){
    printf("%s v%s\n", CLIENT_NAME, VERSION);   
}
