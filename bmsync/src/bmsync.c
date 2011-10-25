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

/*
Contains the entry-point for the bmsync command-line utility.
*/

static struct SyncPrefs prefs = {0, 0, NULL, 0, DEFAULT_PORT, NULL, NULL};
static SOCKET doConnect(char* host, char* alias, int port);

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
