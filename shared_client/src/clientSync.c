/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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
 */

#include <sqlite3.h>
#include "common.h"

/*
Contains a helper function for use by clients that need to retrieve data to be synchronized with another database.
*/

#define CLIENT_SYNC_SQL "SELECT ts AS ts, dl AS dl, ul AS ul, dr AS dr, ad AS ad FROM data WHERE ts > ? AND hs = '' ORDER BY ts ASC"

struct Data* getSyncValues(time_t ts){
 // A list of Data structs will be returned, once for each db entry with a timestamp > ts

   	sqlite3_stmt *stmt = getStmt(CLIENT_SYNC_SQL);

	sqlite3_bind_int(stmt, 1, ts);
	struct Data* result = runSelect(stmt);

	finishedStmt(stmt);

	return result;
}
