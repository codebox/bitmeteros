#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include "common.h"
#include "bmws.h"
#ifndef _WIN32
	#include <netinet/in.h>
#endif

#define DEFAULT_PORT 2605
#define LOCAL_ONLY htonl(0x7f000001L)
#define CONNECTION_BACKLOG 10

static void setCustomLogLevel();

int getPort(){
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
            logMsg(LOG_ERR, "The db config value %s contained an invalid port number of %d, using default of %d instead.",
                    CONFIG_WEB_PORT, configValue, DEFAULT_PORT);
            return DEFAULT_PORT;
        }
    }
}

int isLocalConnection(SOCKET socket){
    struct sockaddr_in sa;
    int sa_len = sizeof(sa);
    if (getsockname(socket, (struct sockaddr*)&sa, &sa_len) == -1) {
        #ifdef _WIN32
            logWin32ErrMsg("getsockname() returned an error.", WSAGetLastError());
        #else
            logMsg(LOG_ERR, "getsockname() returned an error: %s", strerror(errno));
        #endif
        
        return FALSE;
    }
 // Local access means any IP in the 127.x.x.x range
    return (sa.sin_addr.s_addr & 0xff) == 127;
}

static char webRoot[MAX_PATH_LEN];
void getWebRoot(char* path){
    strcpy(path, webRoot);
}

struct WebConnectionConfig readDbConfig(){
 // Read in some config parameters from the database
    OPEN_DB();
    setCustomLogLevel();
    dbVersionCheck();
    
    int allowRemote = getConfigInt(CONFIG_WEB_ALLOW_REMOTE, 0);
    int allowRemoteConnect = (allowRemote >= ALLOW_REMOTE_CONNECT);
    int allowRemoteAdmin   = (allowRemote == ALLOW_REMOTE_ADMIN);

    int port = getPort();
    getWebRootPath(webRoot); 
    
    struct WebConnectionConfig webConfig = {port, allowRemoteConnect, allowRemoteAdmin};
    CLOSE_DB();
    
    return webConfig;
}

static void setCustomLogLevel(){
 // If a custom logging level for the web server process has been set in the db then use it
    int dbLogLevel = getConfigInt(CONFIG_WEB_LOG_LEVEL, TRUE);
    if (dbLogLevel > 0) {
        setLogLevel(dbLogLevel);
    }
}

SOCKET setupListener(struct WebConnectionConfig config){
 // Sets up, and returns, the listener socket that we use to receive client requests
    SOCKET listener = socket(AF_INET, SOCK_STREAM,0);
    if (listener == INVALID_SOCKET) {
        #ifdef _WIN32
            logWin32ErrMsg("socket() returned an error.", WSAGetLastError());
        #else
            logMsg(LOG_ERR, "socket() returned %d, %s", listener, strerror(errno));
        #endif
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
    serverAddress.sin_addr.s_addr = (config.allowRemoteConnect ? INADDR_ANY : LOCAL_ONLY);
    serverAddress.sin_port = htons(config.port);

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