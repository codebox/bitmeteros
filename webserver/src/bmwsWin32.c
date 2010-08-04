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
static int allowRemoteConnect, allowRemoteAdmin;
static int isLocalConnection(SOCKET socket);

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
    	int allowAdmin = isLocalConnection(fd) || allowRemoteAdmin;
    	processRequest(fd, buffer, allowAdmin);	
    }
	
	closesocket(fd);
	_endthread();

}

static int port;
static int getPort(){
 // This returns the TCP port that we listen on
    int configValue;
    if ((configValue = getConfigInt(CONFIG_WEB_PORT, TRUE)) < 0){
     /* If a value hasn't been specified in the db config table then we just use the default. */
        return DEFAULT_PORT;

    } else {
     // A value was specified in the db, so use that port instead, assuming it is valid.
        if (configValue >= MIN_PORT && configValue <= MAX_PORT){
            return configValue;

        } else {
            logMsg(LOG_ERR, "The db config value %s contained an invalid port number of %s, using default of %d instead.",
                    CONFIG_WEB_PORT, configValue, DEFAULT_PORT);
            return DEFAULT_PORT;
        }
    }
}

static char webRoot[MAX_PATH_LEN];
void getWebRoot(char* path){
    strcpy(path, webRoot);
}

static void setCustomLogLevel(){
 // If a custom logging level for the web server process has been set in the db then use it
	int dbLogLevel = getConfigInt(CONFIG_WEB_LOG_LEVEL, TRUE);
	if (dbLogLevel > 0) {
		setLogLevel(dbLogLevel);
	}
}

static void readDbConfig(){
 // Read in some config parameters from the database
	openDb();
	setCustomLogLevel();
	dbVersionCheck();
	
	int allowRemote = getConfigInt(CONFIG_WEB_ALLOW_REMOTE, 0);
	allowRemoteConnect = (allowRemote >= ALLOW_REMOTE_CONNECT);
	allowRemoteAdmin   = (allowRemote == ALLOW_REMOTE_ADMIN);

	port = getPort();
    getWebRootPath(webRoot);	
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
	serverAddress.sin_port = htons(port);

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

int isLocalConnection(SOCKET socket){
	struct sockaddr_in sa;
	int sa_len = sizeof(sa);
	if (getsockname(socket, &sa, &sa_len) == -1) {
		logWin32ErrMsg("getsockname() returned an error.", WSAGetLastError());
		return FALSE;
	}
 // Local access means any IP in the 127.x.x.x range
	return (sa.sin_addr.s_addr & 0xff) == 127;
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

