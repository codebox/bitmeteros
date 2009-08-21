#include <stdio.h>
#include <stdlib.h>
#include "bmclient.h"
#include "common.h"

extern char* helpTxt;

void doHelp(){
	doVersion();
	printf(helpTxt);
}

void doVersion(){
	printf("%s v%s\n", CLIENT_NAME, VERSION);	
}
