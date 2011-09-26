#include "common.h"
#include "bmdb.h"
#include "pcap.h"
#include <stdio.h>

/*
Displays some pieces of version information which might be useful.
*/
int doVersion(){
	printf(
		"App Version:    %s\n"
		"DB Version:     %d\n"
		"SQLite Version: %s\n"
		"PCAP Version:   %s\n",
		VERSION,
		getDbVersion(),
		sqlite3_libversion(),
		pcap_lib_version());

    return SUCCESS;
}
