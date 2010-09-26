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

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
	#define WINVER 0x0501
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <netdb.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"
#include "capture.h"
#include "bmsync.h"
#include "bmws.h"
#include "sqlite3.h"

#define MAX_REQUEST_LEN   1024
#define DEFAULT_PORT 2605
#define DEFAULT_HTTP_PORT 80
#define MAX_ADDR_LEN 64
#define MAX_LINE_LEN 128

#define MSG_CONNECTING "Connecting..."
#define MSG_CONNECTED  "Connected"

/*
Contains the entry-point for the bmsync command-line utility.
*/

static struct SyncPrefs prefs = {0, 0, NULL, 0, DEFAULT_PORT, NULL, NULL};
static SOCKET doConnect(char* host, char* alias, int port);
static int parseData(SOCKET fd, char* alias, int*);
static struct Data* parseRow(char* row);
static int sendRequest(SOCKET fd, long ts, char* host, int port);
static time_t getMaxTsForHost(char* alias);

int main(int argc, char **argv){
    printf(COPYRIGHT);
    fflush(stdout);
    setLogLevel(LOG_INFO);
    setAppName("bmsync");
    
	int status = parseSyncArgs(argc, argv, &prefs);

	if (status == FAIL){
	 // The command-line was duff...
		if (prefs.errMsg != NULL){
		 // ...and we have a specific error to show the user
			logMsg(LOG_ERR, "Error: %s\n", prefs.errMsg);
		} else {
		 // ...and we have no specific error message, so show a vague one
			logMsg(LOG_ERR, "bmsync did not understand.");
		}
		logMsg(LOG_INFO, "Use the '-h' option to display help.\n");

	} else if (prefs.version == TRUE) {
	    doVersion();

	} else if (prefs.help == TRUE) {
	    doHelp();

	} else {
		openDb();
		dbVersionCheck();
		setupDb();

		int i;
		SOCKET sockFd;
		char* host;
		char* alias;
		int port = prefs.port;

		#ifdef _WIN32
			WSADATA wsaData;
			int rc = WSAStartup(MAKEWORD(2,2), &wsaData);
	    	if (rc != 0) {
		        logMsg(LOG_ERR, "WSAStartup returned error %d", rc);
		        exit(1);
		    }
		#endif

		for (i=0; i<prefs.hostCount; i++) {
		    resetStatusMsg();
			host  = prefs.hosts[i];
			alias = (prefs.alias == NULL) ? host : prefs.alias;

		 // We want one transaction per host, insertion of all the data for a single host must be atomic
			beginTrans(TRUE);

            time_t ts = getMaxTsForHost(alias);

		 // Attempt to connect to the host/port specified
			sockFd = doConnect(host, alias, port);
			if (sockFd != FAIL){
				statusMsg(MSG_CONNECTED);

			 // We're in - send a request for the data
			 	int sendResult = sendRequest(sockFd, ts, host, port);
			 	if (sendResult == SUCCESS){
			 	 // Parse the response
                    int rowCount;
					int parseResult = parseData(sockFd, alias, &rowCount);

					if (parseResult == SUCCESS){
                     // It all worked ok, commit the transaction
						commitTrans();
						statusMsg("%d new row%s", rowCount, (rowCount == 1 ? "" : "s"));

					} else {
                     // Something was wrong with the response, end the transaction
                     	logMsg(LOG_ERR, "unable to parse sync response row");
						rollbackTrans();
					}

				} else {
                 // Unable to send the request to the remote host, end the transaction
                 	logMsg(LOG_ERR, "unable to send sync request to %s:%d", host, port);
					rollbackTrans();
				}
                close(sockFd);

			} else {
             // Unable to connect, end the transaction
             	logMsg(LOG_ERR, "failed to connect to %s:%d", host, port);
				rollbackTrans();
			}
			printf("\n");
		}
		closeDb();
	}

	#ifdef _WIN32
		WSACleanup();
	#endif

	return 0;
}

static time_t getMaxTsForHost(char* alias){
 // Look for rows in the local database for this particular alias, return the ts value of the newest one (or 0)
 	sqlite3_stmt* stmt = getStmt("SELECT MAX(ts) AS ts FROM data WHERE hs = ?");
	sqlite3_bind_text(stmt, 1, alias, -1, SQLITE_TRANSIENT);
    struct Data* result = runSelect(stmt);
    finishedStmt(stmt);

    time_t ts;
    if (result == NULL){
     // No rows in the table for this host yet
        ts = 0;
    } else {
     // We have seen this host before
        ts = result->ts;
        assert(result->next == NULL);
    }

    return ts;
}

static int sendRequest(SOCKET fd, time_t ts, char* host, int port){
 // Send an HTTP request to the specified host/port asking for any data newer than 'ts'
	char buffer[MAX_REQUEST_LEN];
    sprintf(buffer, "GET /sync?ts=%d HTTP/1.1" HTTP_EOL, (int)ts);
    send(fd, buffer, strlen(buffer), 0);

	if (port == DEFAULT_HTTP_PORT){
		sprintf(buffer, "Host: %s", host);
	} else {
		sprintf(buffer, "Host: %s:%d", host, port);
	}
	sprintf(buffer, HTTP_EOL HTTP_EOL);

    send(fd, buffer, strlen(buffer), 0);

    return SUCCESS;
}

static struct Data* parseRow(char* row){
 // Parse a single row from the response into a Data struct
	char addr[MAX_ADDR_LEN];
	struct Data* data = allocData();

 // Row format is: ts,dr,dl,ul,ad
	sscanf(row, "%d,%d,%llu,%llu,%s", (int*)&(data->ts), &(data->dr),
		&(data->dl), &(data->ul), addr);
	setAddress(data, addr);

	return data;
}

static int startsWith(char* txt, char* start){
    char *ptr = strstr(txt, start);
    return ptr == txt;
}

static int readLine(SOCKET fd, char* line){
    int prevChar = 0, thisChar = 0;
    int lineIndex = 0;
    int rc;
    while ((rc = recv(fd, (line + lineIndex), 1, 0)) > 0) {
        thisChar = line[lineIndex++];
        if (thisChar== '\n' && prevChar == '\r'){
         // We found an HTTP end-of-line sequence, so the current line has been read. Insert a string terminator and return.
        	line[lineIndex] = 0;
            return TRUE;

        } else {
         // Not at the end of a line yet, keep reading
            prevChar = thisChar;
        }

        if (lineIndex >= MAX_LINE_LEN){
         /* This is unlikely to have come from the BitMeter server, possibly a long header added
            in by a proxy en-route, we can just truncate it and continue */
            line[MAX_LINE_LEN] = 0;
            return TRUE;
        }
    }
    if (rc < 0 && errno > 0){
     // There was a problem reading from the socket
    	statusMsg("ERR %d %d %s\r\n", rc, errno, strerror(errno));
    }

	return FALSE;
}

static int parseData(SOCKET fd, char* alias, int* rowCount){
 // Handle the response that is returned from the host
	int result = SUCCESS;
	char line[MAX_LINE_LEN + 1];

 // First read all the HTTP headers
	while(readLine(fd, line)) {
	    if (startsWith(line, "HTTP")) {
         // Check the server returned an HTTP 200 response code
            char responseCode[4];
            sscanf(line, "%*s %s %*s", responseCode);
            if (strcmp("200", responseCode) != 0){
                statusMsg("Bad HTTP response code: %s", responseCode);
                result = FAIL;
                break;
            }

	    } else if (startsWith(line, HEADER_CONTENT_TYPE)) {
         // Check that the content-type is what we expect
            if (strstr(line, SYNC_CONTENT_TYPE) == NULL){
                statusMsg("Bad content type: %s", line);
                result = FAIL;
                break;
            }

	    } else if (startsWith(line, HTTP_EOL)) {
         // Reached the end of the headers
            break;
	    }
	}

	if (result == FAIL){
     // There was a problem with the headers, so stop now
        return FAIL;
	}

 // Read the data one row at a time
	struct Data* data;
	int resultCount = 0, rc;

 	while(readLine(fd, line)) {
        data = parseRow(line);
        if (data == NULL){
            statusMsg("Malformed data returned from host");
            result = FAIL;
            break;
        }

        setHost(data, alias);
        rc = insertData(data);
        if (rc == FAIL){
            statusMsg("Unable to insert data into local db");
            result = FAIL;
            break;
        }
        freeData(data);
        resultCount++;
	}
	*rowCount = resultCount;

	return result;
}

static SOCKET doConnect(char* host, char* alias, int port){
	struct addrinfo hints, *res;
	SOCKET sockfd;

    printf("Synchronising with %s [%s]: ", host, alias);
    statusMsg(MSG_CONNECTING);

	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	char portTxt[6];
	sprintf(portTxt, "%d", port);

	int rc = getaddrinfo(host, portTxt, &hints, &res);
	if (rc != 0){
    	statusMsg(gai_strerror(rc));
    	return FAIL;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1){
    	statusMsg(strerror(errno));
    	return FAIL;
	}

	rc = connect(sockfd, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);

	if (rc < 0){
    	statusMsg(strerror(errno));
    	close(sockfd);
    	return FAIL;
	} else {
		return sockfd;
	}

}
