#include "common.h"
#include "bmdb.h"
#include "pcap.h"
#include <stdio.h>

/*
Displays some pieces of version information which might be useful.
*/
int doVersion(){
	setTextColour(TEXT_YELLOW);
	printf("App Version:    ");
	setTextColour(TEXT_DEFAULT);
	printf(VERSION);
	
	setTextColour(TEXT_YELLOW);
	printf(EOL "DB Version:     ");
	setTextColour(TEXT_DEFAULT);
	printf("%d", getDbVersion());
	
	setTextColour(TEXT_YELLOW);
	printf(EOL "SQLite Version: ");
	setTextColour(TEXT_DEFAULT);
	printf(sqlite3_libversion());
	
	setTextColour(TEXT_YELLOW);
	printf(EOL "PCAP Version:   ");
	setTextColour(TEXT_DEFAULT);
	printf(pcap_lib_version());
	printf(EOL);

    return SUCCESS;
}
