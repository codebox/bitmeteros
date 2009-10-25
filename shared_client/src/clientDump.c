/*
 * BitMeterOS v0.1.5
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
 * Build Date: Sun, 25 Oct 2009 17:18:38 +0000
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include "common.h"
#include "client.h"

/*
Contains a thread-safe helper function for use by clients that need to performs database dumps
*/

static sqlite3_stmt *stmt = NULL;
static pthread_mutex_t stmtMutex = PTHREAD_MUTEX_INITIALIZER;

void getDumpValues(void (*callback)(struct Data*)){
 // The callback function gets invoked once for each row in the 'data' table
    pthread_mutex_lock(&stmtMutex);

    if (stmt == NULL){
         prepareSql(&stmt, "SELECT ts AS ts, dr AS dr, dl AS dl, ul AS ul, ad AS ad FROM data ORDER BY ts DESC");
    }

    runSelectAndCallback(stmt, callback);
    pthread_mutex_unlock(&stmtMutex);
}

