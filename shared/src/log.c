#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "common.h"

/*
Contains logging functions.
*/
int logToFile = FALSE;
void setLogToFile(int toFile){
 // Determine whether we write log messages to a file, or to stdout
	logToFile = toFile;
}

static int logLevel;
void setLogLevel(int level){
 // Determine logging verbosity here
	logLevel = level;
}
int isLogDebug(){
	return logLevel == LOG_DEBUG;	
}
int isLogInfo(){
	return (logLevel == LOG_DEBUG) || (logLevel == LOG_INFO);	
}

static char* appName = NULL;
void setAppName(const char* thisAppName){
 // Set app name as it will appear in the log entries
	appName = strdup(thisAppName);
}
void logMsg(int level, char* msg, ...){
	va_list argp;
	va_start(argp, msg);
	vlogMsg(level, msg, argp);
	va_end(argp);	
}
void vlogMsg(int level, char* msg, va_list argp){
 // Log the specified message, if its level is high enough
	if (level >= logLevel){
	 // The level (importance) of this message is sufficiently high that we want to log it
        FILE* logFile;
        if (logToFile == TRUE){
            char* logPath = malloc(MAX_PATH_LEN);
            getLogPath(logPath);
            logFile = fopen(logPath, "a+");

            if (logFile == NULL){
             // The specified log file can't be accessed
                logToFile = FALSE;
                logMsg(LOG_ERR, "Unable to log to the file %s, logging to stdout instead", logPath);
                free(logPath);
                logMsg(level, msg); // log the original message
                return;
            }
            free(logPath);

        } else {
            if (level <= LOG_INFO){
				logFile = stdout;
			} else {
				logFile = stderr;
			}
        }

        if (logToFile == TRUE){
         // Only write a timestamp if we are logging to a file (as opposed to stdout/err)
            const time_t time = (time_t) getTime();
            struct tm* cal = localtime(&time);
            int y  = 1900 + cal->tm_year;
            int mo = 1 + cal->tm_mon;
            int d  = cal->tm_mday;
            int h  = cal->tm_hour;
            int mi = cal->tm_min;
            int s  = cal->tm_sec;

            fprintf(logFile, "%d-%02d-%02d %02d:%02d:%02d ", y, mo, d, h, mi, s);
            if (appName != NULL){
                fprintf(logFile, appName);
                fprintf(logFile, " ");
            }
        }

	 // Write out the message, substituting the optargs for any tokens in the text
		vfprintf(logFile, msg, argp);
		fprintf(logFile, EOL);

		fflush(logFile);

		if (logToFile == TRUE){
			fclose(logFile);
		}
	}
}

static int lastStatusMsgLen = 0;
void statusMsg(const char* msg, ...){
    int i;
    for(i=0; i<lastStatusMsgLen; i++){
        printf("\b \b");
    }

 // Write out the message, substituting the optargs for any tokens in the text
    va_list argp;
    va_start(argp, msg);
    lastStatusMsgLen = vfprintf(stdout, msg, argp);
    va_end(argp);
    fflush(stdout);
}
void resetStatusMsg(){
    lastStatusMsgLen = 0;
}
