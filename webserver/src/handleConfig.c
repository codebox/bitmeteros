/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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

#include <stdlib.h>
#include <stdio.h>
#include "bmws.h"
#include "client.h"
#include "common.h"

#define HOST_ADAPTER_SQL "SELECT hs AS hs, ad AS ad FROM data GROUP BY hs, ad"

/*
Handles '/config' requests received by the web server.
*/

static void writeNumConfigValue(SOCKET fd, char* key, char* value);
static void writeTxtConfigValue(SOCKET fd, char* key, char* value);
static void writeHostAdapterList(SOCKET fd);
extern struct HttpResponse HTTP_OK;

void processConfigRequest(SOCKET fd, struct Request* req){
 // Write the JSON object out to the stream
    writeHeaders(fd, HTTP_OK, MIME_JS, TRUE);

	writeText(fd, "var config = { ");
	char* val = getConfigText(CONFIG_WEB_MONITOR_INTERVAL, FALSE);
    writeNumConfigValue(fd, "monitorInterval", val);
    free(val);

    writeText(fd, ", ");
    val = getConfigText(CONFIG_WEB_SUMMARY_INTERVAL, FALSE);
    writeNumConfigValue(fd, "summaryInterval", val);
    free(val);

    writeText(fd, ", ");
    val = getConfigText(CONFIG_WEB_HISTORY_INTERVAL, FALSE);
    writeNumConfigValue(fd, "historyInterval", val);
    free(val);

    writeText(fd, ", ");
    val = getConfigText(CONFIG_WEB_SERVER_NAME, FALSE);
    writeTxtConfigValue(fd, "serverName", val);
    free(val);

    writeText(fd, ", ");
    val = getConfigText(CONFIG_WEB_COLOUR_DL, FALSE);
    writeTxtConfigValue(fd, "dlColour", val);
    free(val);

    writeText(fd, ", ");
    val = getConfigText(CONFIG_WEB_COLOUR_UL, FALSE);
    writeTxtConfigValue(fd, "ulColour", val);
    free(val);

    writeText(fd, ", ");
    writeTxtConfigValue(fd, "version", VERSION);

    writeText(fd, ", ");
    writeHostAdapterList(fd);

	writeText(fd, " };");
}

static void writeNumConfigValue(SOCKET fd, char* key, char* value){
 // Helper function, writes a key/value pair to the stream
    char txt[64];
    sprintf(txt, "'%s' : %s", key, value);
    writeText(fd, txt);
}

static void writeTxtConfigValue(SOCKET fd, char* key, char* value){
 // Helper function, writes a key/value pair to the stream surrounding the value with quotes
    char txt[64];
    sprintf(txt, "'%s' : '%s'", key, value);
    writeText(fd, txt);
}

static void writeHostAdapterList(SOCKET fd){
    sqlite3_stmt* stmt;

    prepareSql(&stmt, HOST_ADAPTER_SQL);
    struct Data* result = runSelect(stmt);
    struct Data* currentResult = result;

    writeText(fd, "'adapters' : [");
    int firstResult = TRUE;
    while(currentResult != NULL){
        if (firstResult == FALSE){
            writeText(fd, ",");
        }

        writeText(fd, "{");
        if (strcmp("", currentResult->hs) == 0){
            writeTxtConfigValue(fd, "hs", "local");
        } else {
            writeTxtConfigValue(fd, "hs", currentResult->hs);
        }
        writeText(fd, ",");
        writeTxtConfigValue(fd, "ad", currentResult->ad);
        writeText(fd, "}");

        firstResult = FALSE;
        currentResult = currentResult->next;
    }
    writeText(fd, "]");

    sqlite3_finalize(stmt);
    freeData(result);
}
