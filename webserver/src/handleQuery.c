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

#include <stdlib.h>
#include "sqlite3.h"
#include "bmws.h"
#include "common.h"
#include "client.h"

#define BAD_PARAM -1

/*
Handles '/query' requests received by the web server.
*/

extern struct HttpResponse HTTP_OK;
extern struct HttpResponse HTTP_SERVER_ERROR;

void processQueryRequest(SOCKET fd, struct Request* req){
	struct NameValuePair* params = req->params;

	time_t from  = (time_t) getValueNumForName("from",  params,  BAD_PARAM);
	time_t to    = (time_t) getValueNumForName("to",    params,  BAD_PARAM);
	long group = getValueNumForName("group", params,  BAD_PARAM);

    if (from == BAD_PARAM || to == BAD_PARAM || group == BAD_PARAM){
     // We need all 3 parameters
        writeHeaders(fd, HTTP_SERVER_ERROR, NULL, 0);

    } else {
        writeHeaders(fd, HTTP_OK, MIME_JSON, 0);

        if (from > to){
         // Allow from/to values in either order
            time_t tmp = from;
            from = to;
            to = tmp;
        }

     /* The client will send a 'from' ts that corresponds to the start of the first day of the range (ie 00:00:00)
        we need to adjust this to 01:00:00, which will be the ts value held in the database that covers the time
        range from 00:00:00 to 01:00:00. */
        struct tm* cal = gmtime(&from);
        cal->tm_hour++;
        from = mktime(cal);

     /* The client will send the last date that should be included in the query range in the 'to' parameter. When
        computing the timestamp value that corresponds to this, we need to move to the end of the specified date to
        be sure of including all data transferred during that day. */
        cal = gmtime(&to);
        cal->tm_mday++;
        to = mktime(cal);

        struct Data* result = getQueryValues(from, to, group);
        writeDataToJson(fd, result);
        freeData(result);
    }

}
