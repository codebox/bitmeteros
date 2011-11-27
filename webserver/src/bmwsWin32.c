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

#define MIN_PORT 1
#define MAX_PORT 65535

static SOCKET listener;
static struct WebConnectionConfig config;

static void web(void* fdVoid){
    SOCKET fd = (SOCKET) fdVoid;
    long rc;
    char buffer[BUFSIZE+1];

    rc = recv(fd, buffer, BUFSIZE, 0);
    
    if (rc < 0){
        logMsg(LOG_ERR, "recv() returned %d, %s", rc, strerror(errno));
    } else if (rc == 0){
        logMsg(LOG_ERR, "recv() returned 0");
    } else if(rc >= BUFSIZE){
        logMsg(LOG_ERR, "recv() return value indicates request too large for buffer size of %d", BUFSIZE);
    } else {
        int allowAdmin = isLocalConnection(fd) || config.allowRemoteAdmin;
        processRequest(fd, buffer, allowAdmin); 
    }
    
    closesocket(fd);
    _endthread();

}

void setupWeb(){
    setLogLevel(LOG_ERR);
    setAppName("WEB");
    setLogToFile(TRUE);
    initMutex();
    
    config = readDbConfig();
    
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
    
    listener = setupListener(config);
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

