#include <stdio.h>
#include <stdlib.h>
#include "bmsync.h"
#include "common.h"

/*
Contains the code that handles help and version requests made via the bmsync utility.
Help text is read from the helpText.c file which is generated during the build process.
*/

extern char* helpTxt;

void doHelp(){
	printf(helpTxt);
}

void doVersion(){
	printf("%s v%s\n", SYNC_NAME, VERSION);
}
