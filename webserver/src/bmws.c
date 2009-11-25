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
#define DEFAULT_PORT 2605
#define MIN_PORT 1
#define MAX_PORT 65535
#define LOCAL_ONLY htonl(0x7f000001L)

static void sigHandler();
static int allowRemoteConnect;

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
    } else if(rc >= BUFSIZE){
        logMsg(LOG_ERR, "read() returned %d which is larger than buffer size of %d", rc, BUFSIZE);
        exit(1);
    }

	processRequest(fd, buffer);

    exit(0);
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
	serverAddress.sin_port = htons(getPort());

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

static void readDbConfig(){
 // Read in some config parameters from the database
	openDb();
	prepareDb();
	dbVersionCheck();
	allowRemoteConnect = getConfigInt(CONFIG_WEB_ALLOW_REMOTE);
	closeDb();
}

SOCKET listener;
int main(){
    int pid;
	SOCKET socketfd;
    socklen_t length;
    static struct sockaddr_in clientAddress;

	setLogLevel(LOG_ERR);
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
