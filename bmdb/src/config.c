#include "common.h"
#include "bmdb.h"
#include <stdio.h>

/*
Displays a list of all the configuration values stored in the database.
*/

int doListConfig(FILE* file, int argc, char** argv){
    int rc;
    sqlite3_stmt *stmt;
    prepareSql(&stmt, "SELECT key, value FROM config");

    const unsigned char *key, *value;

    printf(INFO_DUMPING_CONFIG EOL);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        key   = sqlite3_column_text(stmt, 0);
        value = sqlite3_column_text(stmt, 1);
        PRINT(BMDB_COL_2, key);
        PRINT(BMDB_COL_1, "=%s" EOL, value);
    }
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE){
        logMsg(LOG_ERR, " sqlite3_step returned %d in doConfig, %s", rc, getDbError());
        return FAIL;
    } else {
        return SUCCESS;
    }
}

int doSetConfig(FILE* file, int argc, char** argv){
    int status;

    if (argc == 2){
        status = setConfigTextValue(argv[0], argv[1]);
        if (status == SUCCESS){
            PRINT(COLOUR_DEFAULT, "Config value '%s' set to '%s'." EOL, argv[0], argv[1]);
        } else {
            PRINT(COLOUR_RED, "Error - failed to set config value." EOL);
        }
    } else {
        PRINT(COLOUR_RED, "Error - expected 2 arguments, the name and value of the config parameter.\n");
        status = FAIL;
    }
    return status;
}

int doRmConfig(FILE* file, int argc, char** argv){
    int status;
    if (argc == 1){
        status = rmConfigValue(argv[0]);
        if (status == SUCCESS){
            printf("Config value '%s' was removed." EOL, argv[0]);
        } else {
            PRINT(COLOUR_RED, "Error - failed to remove config value." EOL);
        }
    } else {
        PRINT(COLOUR_RED, "Error - expected 1 argument, the name of the config parameter to be removed.\n");
        status = FAIL;
    }
    return status;
}
