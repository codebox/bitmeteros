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
static struct WebConnectionConfig config;

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
    
    int allowAdmin = isLocalConnection(fd) || config.allowRemoteAdmin;
    processRequest(fd, buffer, allowAdmin);

    exit(0);
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
    signal(SIGINT,  sigHandler);    // Trap Ctrl-C in case we are running interactively
    signal(SIGTERM, sigHandler);    // Trap termination requests from the system

    config = readDbConfig();

    listener = setupListener(config);

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
