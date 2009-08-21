#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "common.h"

static char* logPath;
void setLogToFile(int logToFile){
	if (logToFile){
		logPath = malloc(MAX_PATH_LEN);
		getLogPath(logPath);
	} else {
		logPath	= NULL;
	}
}

static int logLevel;
void setLogLevel(int level){
	logLevel = level;
}

int getTime();
void logMsg(int level, char* msg, ...){
	if (level >= logLevel){
		FILE* logFile;
		if (logPath == NULL){
			logFile = stderr;
		} else {
			logFile = fopen(logPath, "a+");
		}

		const time_t time = (time_t) getTime();
		struct tm* cal = localtime(&time);
		int y  = 1900 + cal->tm_year;
		int mo = 1 + cal->tm_mon;
		int d  = cal->tm_mday;
		int h  = cal->tm_hour;
		int mi = cal->tm_min;
		int s  = cal->tm_sec;

		fprintf(logFile, "%d-%02d-%02d %02d:%02d:%02d ", y, mo, d, h, mi, s);

		va_list argp;
		va_start(argp, msg);
		vfprintf(logFile, msg, argp);
		va_end(argp);

		fflush(logFile);

		if (logPath != NULL){
			fclose(logFile);
		}
	}
}
