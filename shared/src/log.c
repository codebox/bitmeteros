/*
 * BitMeterOS v0.3.2
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
 *
 * Build Date: Sun, 07 Mar 2010 14:49:47 +0000
 */

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

static char* logPath;
void setLogToFile(int logToFile){
 // Determine whether we write log messages to a file, or to stdout
	if (logToFile){
		logPath = malloc(MAX_PATH_LEN);
		getLogPath(logPath);
	} else {
		logPath	= NULL;
	}
}

static int logLevel;
void setLogLevel(int level){
 // Determine logging verbosity here
	logLevel = level;
}

static char* appName = NULL;
void setAppName(const char* thisAppName){
 // Set app name as it will appear in the log entries
	appName = strdup(thisAppName);
}

void logMsg(int level, char* msg, ...){
 // Log the specified message, if its level is high enough
	if (level >= logLevel){
	 // The level (importance) of this message is sufficiently high that we want to log it
		FILE* logFile;
		if (logPath == NULL){
			if (level <= LOG_INFO){
				logFile = stdout;
			} else {
				logFile = stderr;
			}

		} else {
			logFile = fopen(logPath, "a+");
		}

        if (logPath != NULL){
         // Write out the timestamp first
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
		va_list argp;
		va_start(argp, msg);
		vfprintf(logFile, msg, argp);
		fprintf(logFile, EOL);
		va_end(argp);

		fflush(logFile);

		if (logPath != NULL){
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
