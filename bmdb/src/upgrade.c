/*
 * BitMeterOS v0.2.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build Date: Wed, 25 Nov 2009 10:48:23 +0000
 */

#define _GNU_SOURCE
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
        logMsg(LOG_INFO, "Please specify a valid database level for the upgrade, the database is currently at level %d.", currentLevel);
		status = FAIL;

	} else if (requestLevel < currentLevel){
     // We were asked to 'upgrade' to an older version
		logMsg(LOG_INFO, "Cannot upgrade database to level %d, the database is already at level %d.", requestLevel, currentLevel);
		status = FAIL;

	} else if (requestLevel == currentLevel){
     // We were asked to 'upgrade' to the current version
		logMsg(LOG_INFO, "Database is already at level %d, nothing to do.", requestLevel);
		status = FAIL;

	} else if (requestLevel > MAX_UPGRADE_LEVEL){
     // We were asked to upgrade to a version that is unknown to this version of the utility
		logMsg(LOG_INFO, "Cannot upgrade database to level %d, the maximum available upgrade level is %d.", requestLevel, MAX_UPGRADE_LEVEL);
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
	beginTrans();
	switch (level){
		case 2:
			status = upgrade2();
			break;
		default:
			assert(FALSE);
	}

	if (status == SUCCESS){
		commitTrans();
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
    prepareSql(&stmtSelect, "SELECT DISTINCT ad FROM data");
    prepareSql(&stmtUpdate, "UPDATE data SET ad=? WHERE ad=?");

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
	sqlite3_reset(stmtSelect);

    struct BinaryAddress* curr = addrList;
 // Go through each of the BinaryAddress structs we populated earlier and run some SQL to convert the ad values to hex strings
    while(curr != NULL){
        sqlite3_bind_text(stmtUpdate, 1, curr->txtData, -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmtUpdate, 2, curr->binData, curr->binDataLength,   SQLITE_TRANSIENT);

        rc = sqlite3_step(stmtUpdate);
        if (rc != SQLITE_DONE){
            logMsg(LOG_ERR, "address update failed rc=%d error=%s", rc, getDbError());
            return FAIL;
        }

        sqlite3_reset(stmtUpdate);
        curr = curr->next;
    }

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
