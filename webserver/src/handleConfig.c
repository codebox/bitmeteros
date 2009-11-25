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
#include <stdio.h>
#include "bmws.h"
#include "client.h"
#include "common.h"

/*
Handles '/config' requests received by the web server.
*/

static void writeConfigValue(SOCKET fd, char* key, char* value);
extern struct HttpResponse HTTP_OK;

void processConfigRequest(SOCKET fd, struct Request* req){
 // Write the JSON object out to the stream
    writeHeaders(fd, HTTP_OK, MIME_JSON, 0);

	writeText(fd, "var config = { ");

	char* val = getConfigText(CONFIG_WEB_MONITOR_INTERVAL);
    writeConfigValue(fd, "monitorInterval", val);
    free(val);

    writeText(fd, ", ");
    val = getConfigText(CONFIG_WEB_SUMMARY_INTERVAL);
    writeConfigValue(fd, "summaryInterval", val);
    free(val);

    writeText(fd, ", ");
    val = getConfigText(CONFIG_WEB_HISTORY_INTERVAL);
    writeConfigValue(fd, "historyInterval", val);
    free(val);

    writeText(fd, ", ");
    writeConfigValue(fd, "version", "'" VERSION "'");
	writeText(fd, " };");
}

static void writeConfigValue(SOCKET fd, char* key, char* value){
 // Helper function, writes a key/value pair to the stream
    char txt[64];
    sprintf(txt, "'%s' : %s", key, value);
    writeText(fd, txt);
}
