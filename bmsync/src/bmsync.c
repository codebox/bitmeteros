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

static int syncWithHost(char* host, char* alias, int port){
    resetStatusMsg();
    
    time_t maxTsForHost = getMaxTsForHost(alias);
    
    // Attempt to connect to the host/port specified
    SOCKET sockFd = doConnect(host, alias, port);
    int status = (sockFd == 0) ? FAIL : SUCCESS;
    
    if (status == SUCCESS) {
        statusMsg(MSG_CONNECTED);
        
        // We're in - send a request for the data
         status = sendRequest(sockFd, maxTsForHost, host, port);
         if (status == SUCCESS){
            // Parse the response
            int rowCount;
            status = parseData(sockFd, alias, maxTsForHost, &rowCount);
            
            if (status == SUCCESS){
                // It all worked ok
                statusMsg("%d new row%s", rowCount, (rowCount == 1 ? "" : "s"));
                
            } else {
                // Something was wrong with the response
                 PRINT(COLOUR_RED, "Unable to parse sync response row");
            }
            
        } else {
            // Unable to send the request to the remote host
             PRINT(COLOUR_RED, "Unable to send sync request to %s:%d", host, port);
        }
        
#ifdef _WIN32
        closesocket(sockFd);
#else
        close(sockFd);
#endif
        
    } else {
        // Unable to connect
         PRINT(COLOUR_RED, "Failed to connect to %s:%d", host, port);
    }
    printf("\n");    
    
    return status;
}

int main(int argc, char **argv){
    openDb();
    showCopyright();
    fflush(stdout);
    setLogLevel(LOG_INFO);
    setAppName("bmsync");
    
    int status = parseSyncArgs(argc, argv, &prefs);

    if (status == FAIL){
     // The command-line was duff...
        if (prefs.errMsg != NULL){
         // ...and we have a specific error to show the user
            PRINT(COLOUR_RED, "Error: %s\n", prefs.errMsg);
        } else {
         // ...and we have no specific error message, so show a vague one
            PRINT(COLOUR_RED, "bmsync did not understand.");
        }
        logMsg(LOG_INFO, "Use the '-h' option to display help.\n");

    } else if (prefs.version == TRUE) {
        doVersion();

    } else if (prefs.help == TRUE) {
        doHelp();

    } else {
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
                PRINT(COLOUR_RED, "WSAStartup returned error %d", rc);
                exit(1);
            }
        #endif

        for (i=0; i<prefs.hostCount; i++) {
            char* host  = prefs.hosts[i];
            char* alias = (prefs.alias == NULL) ? host : prefs.alias;
            
            int statusForThisHost = syncWithHost(host, alias, port);
            if (statusForThisHost != SUCCESS){
                status = FAIL; // Carry on even if one host fails
            }
        }
    }
    closeDb();
    
    #ifdef _WIN32
        WSACleanup();
    #endif

    return (status == FAIL) ? 1 : 0;
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
