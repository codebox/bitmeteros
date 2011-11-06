#include "common.h"
#include "bmdb.h"
#include "pcap.h"
#include <stdio.h>

/*
Displays some pieces of version information which might be useful.
*/
int doVersion(){
    PRINT(BMDB_COL_2, "App Version:    ");
    PRINT(BMDB_COL_1, VERSION);
    
    PRINT(BMDB_COL_2, EOL "DB Version:     ");
    PRINT(BMDB_COL_1, "%d", getDbVersion());
    
    PRINT(BMDB_COL_2, EOL "SQLite Version: ");
    PRINT(BMDB_COL_1, sqlite3_libversion());
    
    PRINT(BMDB_COL_2, EOL "PCAP Version:   ");
    PRINT(BMDB_COL_1, pcap_lib_version());
    printf(EOL);

    return SUCCESS;
}
