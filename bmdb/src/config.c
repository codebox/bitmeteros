#ifdef UNIT_TESTING
	#import "test.h"
#endif
#include "common.h"
#include "bmdb.h"
#include <stdio.h>

/*
Displays a list of all the configuration values stored in the database.
*/

int doListConfig(int argc, char** argv){
	int rc;
	sqlite3_stmt *stmt;
	prepareSql(&stmt, "SELECT key, value FROM config");

	const unsigned char *key, *value;

    printf(INFO_DUMPING_CONFIG EOL);
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
		key   = sqlite3_column_text(stmt, 0);
		value = sqlite3_column_text(stmt, 1);
		printf("%s=%s" EOL, key, value);
	}
	sqlite3_finalize(stmt);

	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, " sqlite3_step returned %d in doConfig, %s", rc, getDbError());
		return FAIL;
	} else {
        return SUCCESS;
	}
}

int doSetConfig(int argc, char** argv){
    int status;
    if (argc == 2){
        status = setConfigTextValue(argv[0], argv[1]);
        if (status == SUCCESS){
        	printf("Config value '%s' set to '%s'." EOL, argv[0], argv[1]);
        } else {
	        printf("Error - failed to set config value." EOL);
        }
    } else {
        printf("Error - expected 2 arguments, the name and value of the config parameter.\n");
        status = FAIL;
    }
    return status;
}

int doRmConfig(int argc, char** argv){
    int status;
    if (argc == 1){
        status = rmConfigValue(argv[0]);
        if (status == SUCCESS){
        	printf("Config value '%s' was removed." EOL, argv[0]);
        } else {
	        printf("Error - failed to remove config value." EOL);
        }
    } else {
        printf("Error - expected 1 argument, the name of the config parameter to be removed.\n");
        status = FAIL;
    }
    return status;
}
