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

#include "common.h"
#include "bmdb.h"
#include <stdio.h>

/*
Displays a list of all the configuration values stored in the database.
*/

int doConfig(FILE* file, int argc, char** argv){
	int rc;
	sqlite3_stmt *stmt;
	prepareSql(&stmt, "SELECT key, value FROM config");

	const unsigned char *key, *value;

    fprintf(file, INFO_DUMPING_CONFIG EOL);
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
		key   = sqlite3_column_text(stmt, 0);
		value = sqlite3_column_text(stmt, 1);
		fprintf(file, "%s=%s" EOL, key, value);
	}
	sqlite3_reset(stmt);

	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, " sqlite3_step returned %d in doConfig, %s", rc, getDbError());
		return FAIL;
	} else {
        return SUCCESS;
	}
}
