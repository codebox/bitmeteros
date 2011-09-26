#define _GNU_SOURCE
#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "bmdb.h"

#define BAD_LEVEL -1

/*
Performs database upgrades, which are sometimes required when a new version of the app is installed.
*/

static int upgradeTo(int level);
static int upgrade2();
static int upgrade3();
static int upgrade4();
static int upgrade5();
static int upgrade6();
static int upgrade7();
static int upgrade8();

int doUpgrade(FILE* file, int argc, char** argv){
    int requestLevel;
    if (argc == 0){
     // No level to upgrade to was specified
        requestLevel = BAD_LEVEL;
    } else {
     // Attempt to convert the specified level to an int
        requestLevel = strToInt(argv[0], BAD_LEVEL);
    }
    int status;
	int currentLevel = getDbVersion();

    if (requestLevel == BAD_LEVEL){
     // We don't know what level to upgrade to
        printf("Please specify a valid database level for the upgrade, the database is currently at level %d.", currentLevel);
		status = FAIL;

	} else if (requestLevel < currentLevel){
     // We were asked to 'upgrade' to an older version
		printf("Cannot upgrade database to level %d, the database is already at level %d.", requestLevel, currentLevel);
		status = FAIL;

	} else if (requestLevel == currentLevel){
     // We were asked to 'upgrade' to the current version
		printf("Database is already at level %d, nothing to do.", requestLevel);
		status = FAIL;

	} else if (requestLevel > MAX_UPGRADE_LEVEL){
     // We were asked to upgrade to a version that is unknown to this version of the utility
		printf("Cannot upgrade database to level %d, the maximum available upgrade level is %d.", requestLevel, MAX_UPGRADE_LEVEL);
		status = FAIL;

	} else {
     // Happy with the requested upgrade level
	    assert(currentLevel < requestLevel);

     // We upgrade one level at a time, if any of the upgrades fail then we stop
		int level = currentLevel + 1;
		while(level <= requestLevel){
			status = upgradeTo(level++);
			if (status == FAIL){
                break;
			}
		}
	}

	return status;
}

static int upgradeTo(int level){
 // Upgrade the db to a specific level, which should be exactly 1 level higher than the current version
	assert(getDbVersion() == level - 1);

	int status = FAIL;
 // The upgrade must be atomic - all or nothing
	beginTrans(FALSE);
	switch (level){
		case 2:
			status = upgrade2();
			break;
		case 3:
			status = upgrade3();
			break;
		case 4:
			status = upgrade4();
			break;
		case 5:
			status = upgrade5();
			break;
		case 6:
			status = upgrade6();
			break;
		case 7:
			status = upgrade7();
			break;
		case 8:
			status = upgrade8();
			break;
		default:
			assert(FALSE);
	}

	if (status == SUCCESS){
		commitTrans();
		printf("Database level upgraded to %d.", level);

	} else {
		rollbackTrans();
	}

	return status;
}

static int setDbVersion(int version){
 // Set the db version in the config table of the database
	if (version < 2){
		assert(FALSE);

	} else {
        return setConfigIntValue(CONFIG_DB_VERSION, version);
	}
}

/*
Early versions of the Windows database stored ad values as bytes rather than strings, a poor design choice
which we fix when upgrading to version 2 of the db.
*/
struct BinaryAddress{
    char* binData;
    char* txtData;
    int binDataLength;
    struct BinaryAddress* next;
};

int convertAddrValues(){
 // Convert all 'ad' values to strings
    sqlite3_stmt* stmtSelect;
    sqlite3_stmt* stmtUpdate;
    prepareSql(&stmtSelect, "SELECT DISTINCT ad FROM data2");
    prepareSql(&stmtUpdate, "UPDATE data2 SET ad=? WHERE ad=?");

    const char* adBytes;
    char adTxt[MAC_ADDR_LEN * 2 + 1];
	int rc, adLen;
    struct BinaryAddress* addrList = NULL;
    struct BinaryAddress* newAddr;

	while ((rc = sqlite3_step(stmtSelect)) == SQLITE_ROW){
     // For each unique ad value populate a BinaryAddress struct containing the old value and the new stringified version
        adBytes = sqlite3_column_blob(stmtSelect, 0);
        adLen = sqlite3_column_bytes(stmtSelect, 0);
        makeHexString(adTxt, adBytes, adLen);

        newAddr = malloc(sizeof(struct BinaryAddress));
        newAddr->binData = malloc(adLen);
        memcpy(newAddr->binData, adBytes, adLen);
        newAddr->binDataLength = adLen;
        newAddr->txtData = strdup(adTxt);
        newAddr->next = NULL;

        if (addrList == NULL){
            addrList = newAddr;
        } else {
            struct BinaryAddress* curr = addrList;
            while(curr->next != NULL){
                curr = curr->next;
            }
            curr->next = newAddr;
        }
	}
	sqlite3_finalize(stmtSelect);

    struct BinaryAddress* curr = addrList;
    struct BinaryAddress* next;
 // Go through each of the BinaryAddress structs we populated earlier and run some SQL to convert the ad values to hex strings
    while(curr != NULL){
    	next = curr->next;
        sqlite3_bind_text(stmtUpdate, 1, curr->txtData, -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmtUpdate, 2, curr->binData, curr->binDataLength,   SQLITE_TRANSIENT);

        rc = sqlite3_step(stmtUpdate);
        if (rc != SQLITE_DONE){
            logMsg(LOG_ERR, "address update failed rc=%d error=%s", rc, getDbError());
            return FAIL;
        }

        sqlite3_reset(stmtUpdate);
        free(curr->binData);
        free(curr->txtData);
        free(curr);
        curr = next;
    }
    sqlite3_finalize(stmtUpdate);

    return SUCCESS;
}

static int upgrade2(){
 // Upgrade the db from version 1 to version 2
    int status;

    status = setDbVersion(2);
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigIntValue(CONFIG_WEB_MONITOR_INTERVAL, 1000);
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigIntValue(CONFIG_WEB_SUMMARY_INTERVAL, 10000);
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigIntValue(CONFIG_WEB_HISTORY_INTERVAL, 10000);
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigIntValue(CONFIG_WEB_ALLOW_REMOTE, FALSE);
    if (status == FAIL){
		return FAIL;
	}

	#ifdef _WIN32
        convertAddrValues();
	#endif

	return SUCCESS;
}

static int upgrade3(){
 // Upgrade the db from version 2 to version 3
    int status = setDbVersion(3);
    if (status == FAIL){
		return FAIL;
	}

    sqlite3_stmt* stmtAddColumn;
    prepareSql(&stmtAddColumn, "ALTER TABLE data2 ADD COLUMN hs;");
    status = sqlite3_step(stmtAddColumn);
    if (status != SQLITE_DONE){
        logMsg(LOG_ERR, "add column failed rc=%d error=%s", status, getDbError());
        return FAIL;
    }

    return SUCCESS;
}

static int upgrade4(){
 // Upgrade the db from version 3 to version 4
    int status = setDbVersion(4);
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigTextValue(CONFIG_WEB_SERVER_NAME, "");
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigTextValue(CONFIG_WEB_COLOUR_DL, "#ff0000");
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigTextValue(CONFIG_WEB_COLOUR_UL, "#00ff00");
    if (status == FAIL){
		return FAIL;
	}

    return SUCCESS;
}

static int upgrade5(){
 // Upgrade the db from version 4 to version 5
    int status = setDbVersion(5);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("UPDATE data2 SET hs = '' WHERE hs IS NULL", NULL);
    if (status == FAIL){
		return FAIL;
	}

    return SUCCESS;
}

static int upgrade6(){
 // Upgrade the db from version 5 to version 6
    int status = setDbVersion(6);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("CREATE TABLE alert (id, name, active, bound, direction, amount)", NULL);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("CREATE TABLE interval (id, yr, mn, dy, wk, hr)", NULL);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("CREATE TABLE alert_interval (alert_id, interval_id)", NULL);
    if (status == FAIL){
		return FAIL;
	}

	status = executeSql("CREATE INDEX idxDataTs ON data2(ts)", NULL);
    if (status == FAIL){
		return FAIL;
	}
	
    return SUCCESS;
}

static int upgrade7(){
 // Upgrade the db from version 6 to version 7
    int status = setDbVersion(7);
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigTextValue(CONFIG_WEB_RSS_HOST, "");
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigIntValue(CONFIG_WEB_RSS_FREQ, 1);
    if (status == FAIL){
		return FAIL;
	}

    status = setConfigIntValue(CONFIG_WEB_RSS_ITEMS, 10);
    if (status == FAIL){
		return FAIL;
	}

    return SUCCESS;
}

static int upgrade8(){
 // Upgrade the db from version 7 to version 8
    int status = setDbVersion(8);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("CREATE TABLE filter (id INTEGER PRIMARY KEY AUTOINCREMENT, desc, name, filter, host)", NULL);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("INSERT INTO filter (id,desc,name,filter) VALUES (1, 'All Downloads', 'dl', 'dst host {adapter}')", NULL);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("INSERT INTO filter (id,desc,name,filter) VALUES (2, 'All Uploads', 'ul', 'src host {adapter}')", NULL);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("INSERT INTO filter (desc,name,filter) VALUES ('Internet Downloads', 'idl', 'dst host {adapter} and not (src net {lan})')", NULL);
    if (status == FAIL){
		return FAIL;
	}

    status = executeSql("INSERT INTO filter (desc,name,filter) VALUES ('Internet Uploads', 'iul', 'src host {adapter} and not (dst net {lan})')", NULL);
    if (status == FAIL){
		return FAIL;
	}

//insert into data2 (ts,dr,vl,fl,hs) select ts,dr,dl,1,hs from data;
//insert into data2 (ts,dr,vl,fl,hs) select ts,dr,ul,2,hs from data;

    return SUCCESS;
}