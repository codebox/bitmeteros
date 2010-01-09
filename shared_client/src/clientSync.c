/*
 * BitMeterOS v0.3.0
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
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

#include <sqlite3.h>
#include <pthread.h>
#include "common.h"

/*
TODO
*/

#define CLIENT_SYNC_SQL "SELECT ts AS ts, dl AS dl, ul AS ul, dr AS dr, ad AS ad FROM data WHERE ts > ? AND hs IS NULL ORDER BY ts ASC"

#ifndef MULTI_THREADED_CLIENT
	static sqlite3_stmt *stmt = NULL;
#endif


struct Data* getSyncValues(time_t ts){
 // A list of Data structs will be returned, once for each db entry with a timestamp > ts

    #ifdef MULTI_THREADED_CLIENT
    	sqlite3_stmt *stmt = NULL;
    	prepareSql(&stmt, CLIENT_SYNC_SQL);
    #else
    	if (stmt == NULL){
    		prepareSql(&stmt, CLIENT_SYNC_SQL);
    	}
    #endif

	sqlite3_bind_int(stmt, 1, ts);
	struct Data* result = runSelect(stmt);

    #ifdef MULTI_THREADED_CLIENT
    	sqlite3_finalize(stmt);
    #else
    	sqlite3_reset(stmt);
    #endif

	return result;
}
