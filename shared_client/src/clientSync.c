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

#include <sqlite3.h>
#include <pthread.h>
#include "common.h"

/*
TODO
*/

static sqlite3_stmt *stmt = NULL;

struct Data* getSyncValues(int ts){
 // A list of Data structs will be returned, once for each db entry with a timestamp >= ts

    if (stmt == NULL){
        prepareSql(&stmt, "SELECT ts AS ts, dl AS dl, ul AS ul, dr AS dr, ad AS ad FROM data WHERE ts > ? AND hs IS NULL ORDER BY ts DESC");
    }

	sqlite3_bind_int(stmt, 1, ts);
	struct Data* result = runSelect(stmt);
	sqlite3_reset(stmt);

	return result;
}
