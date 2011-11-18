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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include "common.h"
#include "bmws.h"

#define CONNECTION_BACKLOG 10
#define LOCAL_ONLY htonl(0x7f000001L)

static void sigHandler();
static int allowRemoteConnect, allowRemoteAdmin;
static int isLocalConnection(SOCKET socket);

static void web(SOCKET fd){
 // This gets run after we fork() for the client request
    long rc;
    char buffer[BUFSIZE+1];

    rc = read(fd, buffer, BUFSIZE);
    if (rc < 0){
        logMsg(LOG_ERR, "read() returned %d, %s", rc, strerror(errno));
        exit(1);
    } else if (rc == 0){
        logMsg(LOG_ERR, "read() returned 0");
        exit(1);
    } else if(rc > BUFSIZE){
        logMsg(LOG_ERR, "read() returned %d which is larger than buffer size of %d", rc, BUFSIZE);
        exit(1);
    }
    
	int allowAdmin = isLocalConnection(fd) || allowRemoteAdmin;
	processRequest(fd, buffer, allowAdmin);

    exit(0);
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

static SOCKET setupListener(){
 // Sets up, and returns, the listener socket that we use to receive client requests
	SOCKET listener = socket(AF_INET, SOCK_STREAM,0);
	if (listener < 0){
		logMsg(LOG_ERR, "socket() returned %d, %s", listener, strerror(errno));
		exit(1);
	}

	int yes = 1;
	int rc = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (rc < 0){
		logMsg(LOG_ERR, "setsockopt() returned %d, %s", rc, strerror(errno));
		exit(1);
	}

	static struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = (allowRemoteConnect ? INADDR_ANY : LOCAL_ONLY);
	serverAddress.sin_port = htons(port);

	rc = bind(listener, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	if (rc < 0){
		logMsg(LOG_ERR, "bind() returned %d, %s", rc, strerror(errno));
		exit(1);
	}

	rc = listen(listener, CONNECTION_BACKLOG);
	if (rc < 0){
		logMsg(LOG_ERR, "listen() returned %d, %s", rc, strerror(errno));
		exit(1);
	}

	return listener;
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

int isLocalConnection(SOCKET socket){
	struct sockaddr_in sa;
	int sa_len = sizeof(sa);
	if (getsockname(socket, &sa, &sa_len) == -1) {
		logMsg(LOG_ERR, "getsockname() returned an error: %s", strerror(errno));
		return FALSE;
	}
 // Local access means any IP in the 127.x.x.x range
	return (sa.sin_addr.s_addr & 0xff) == 127;
}

SOCKET listener;
int main(){
    int pid;
	SOCKET socketfd;
    socklen_t length;
    static struct sockaddr_in clientAddress;

	setLogLevel(LOG_WARN);
	setAppName("WEB");
	setLogToFile(TRUE);

    signal(SIGCHLD, SIG_IGN); /* ignore child death */
	signal(SIGINT,  sigHandler);	// Trap Ctrl-C in case we are running interactively
	signal(SIGTERM, sigHandler);	// Trap termination requests from the system

    readDbConfig();

    listener = setupListener();

    while (1){
        length = sizeof(clientAddress);

        socketfd = accept(listener, (struct sockaddr *) &clientAddress, &length);
   		
        if (socketfd < 0){
            logMsg(LOG_ERR, "accept() returned %d, %s", socketfd, strerror(errno));
        } else {
            pid = fork();
            //TODO drop privs after fork
            if (pid == 0){
             // We are in the child process
                close(listener);
                web(socketfd);

            } else if (pid > 0){
             // We are still in the parent process
                close(socketfd);

            } else {
                logMsg(LOG_ERR, "fork() returned %d, %s", pid, strerror(errno));
                exit(1);
            }
        }
    }

    return 0;
}

static void sigHandler(){
 // Shut down the listener and stop
	close(listener);
	exit(0);
}
