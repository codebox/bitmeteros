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

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <process.h>
#include <sys/types.h>
#include "common.h"
#include "bmws.h"

#define CONNECTION_BACKLOG 10
#define DEFAULT_PORT 2605
#define MIN_PORT 1
#define MAX_PORT 65535
#define LOCAL_ONLY htonl(0x7f000001L)

static SOCKET listener;
static int allowRemoteConnect;

static void web(void* fdVoid){
	SOCKET fd = (SOCKET) fdVoid;
    long rc;
    char buffer[BUFSIZE+1];

    rc = recv(fd, buffer, BUFSIZE, 0);
    
    if (rc < 0){
        logMsg(LOG_ERR, "read() returned %d, %s", rc, strerror(errno));
    } else if (rc == 0){
        logMsg(LOG_ERR, "read() returned 0");
    } else if(rc >= BUFSIZE){
        logMsg(LOG_ERR, "read() returned %d which is larger than buffer size of %d", rc, BUFSIZE);
    } else {
    	processRequest(fd, buffer);	
    }
	
	closesocket(fd);
	_endthread();

}

static int getPort(){
 // This returns the TCP port that we listen on
    char* envValue;
    if ((envValue = getenv(ENV_PORT)) == NULL){
     /* If the 'BITMETER_WEB_PORT' environment variable hasn't been set then we just use the default. */
        return DEFAULT_PORT;

    } else {
     // The 'BITMETER_WEB_PORT' environment variable is set, so use that port instead, assuming it is valid.
        int port = strToInt(envValue, 0);
        if (port >= MIN_PORT && port <= MAX_PORT){
            return port;

        } else {
            logMsg(LOG_ERR, "The %s environment variable contained an invalid port number of %s, using default of %d instead.",
                    ENV_PORT, envValue, DEFAULT_PORT);
            return DEFAULT_PORT;
        }
    }
}

static void readDbConfig(){
 // Read in some config parameters from the database
	openDb();
	prepareDb();
	dbVersionCheck();
	allowRemoteConnect = getConfigInt(CONFIG_WEB_ALLOW_REMOTE);
	closeDb();
}

void setupWeb(){
	setLogLevel(LOG_ERR);
	setAppName("WEB");
	setLogToFile(TRUE);
	initMutex();
	readDbConfig();
	
    WSADATA wsaData;
    WORD socketsVersion = MAKEWORD(2, 2);

    int rc = WSAStartup(socketsVersion, &wsaData);
    if (rc != 0) {
        logMsg(LOG_ERR, "WSAStartup returned error %d", rc);
        exit(1);
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        logMsg(LOG_ERR, "Winsock.dll version mismatch");
        WSACleanup();
        exit(1);
    }
    
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET){
        logWin32ErrMsg("socket() returned an error.", WSAGetLastError());
        exit(1);
    }
    
    SOCKADDR_IN serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = (allowRemoteConnect ? INADDR_ANY : LOCAL_ONLY);
	serverAddress.sin_port = htons(getPort());

	rc = bind(listener, (struct sockaddr *) &serverAddress, sizeof(SOCKADDR_IN));
	if (rc == SOCKET_ERROR){
        logWin32ErrMsg("bind() returned an error.", WSAGetLastError());
        exit(1);
	}
	
	rc = listen(listener, CONNECTION_BACKLOG);
	if (rc == SOCKET_ERROR){
        logWin32ErrMsg("listen() returned an error.", WSAGetLastError());
        exit(1);
	}
}

void shutdownWeb(){
 // Shut down the listener and stop
	close(listener);
	WSACleanup();
}

void processWeb(){
    static struct sockaddr_in clientAddress;
    int length = sizeof(clientAddress);

    SOCKET clientSocket = accept(listener, (struct sockaddr *) &clientAddress, &length);

    if (clientSocket == INVALID_SOCKET) {
    	closesocket(listener);
		logWin32ErrMsg("accept() returned an error.", WSAGetLastError());
    	exit(1);

    } else {
    	HANDLE handle = (HANDLE) _beginthread(web, 0, (void*)clientSocket);
    	if (handle < 0){
			logWin32ErrMsg("_beginthread() returned an error.", (int)handle);
        	exit(1);
    	}
    }
}
