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

#ifndef COMMON_H
#define COMMON_H

#include <sqlite3.h>
#include <stddef.h>
#include <time.h>

#ifdef _WIN32
#define EOL "\r\n"
#endif

#ifdef __linux__
#define EOL "\n"
#endif

#ifdef __APPLE__
#define EOL "\n"
#endif

#define VERSION "0.7.5"
#define DB_VERSION 7

#ifdef _WIN32
#define COPYRIGHT "BitMeter OS v" VERSION " Copyright (c) 2011 Rob Dawson" EOL "Licenced under the GNU General Public License" EOL EOL
#else
#define COPYRIGHT "BitMeter OS v" VERSION " Copyright © 2011 Rob Dawson" EOL "Licenced under the GNU General Public License" EOL EOL
#endif

#define DB_NAME      "bitmeter.db"
#define LOG_NAME     "bitmeter.log"
#define OUT_DIR      "BitMeterOS"
#define IN_MEMORY_DB ":memory:"
#define ENV_DB       "BITMETER_DB"

#define CONFIG_DB_VERSION           "db.version"
#define CONFIG_LOG_PATH             "cap.logpath"
#define CONFIG_CAP_LOG_LEVEL        "cap.loglevel"
#define CONFIG_WEB_LOG_LEVEL        "web.loglevel"
#define CONFIG_WEB_PORT             "web.port"
#define CONFIG_WEB_DIR              "web.dir"
#define CONFIG_WEB_MONITOR_INTERVAL "web.monitor_interval"
#define CONFIG_WEB_SUMMARY_INTERVAL "web.summary_interval"
#define CONFIG_WEB_HISTORY_INTERVAL "web.history_interval"
#define CONFIG_WEB_SERVER_NAME      "web.server_name"
#define CONFIG_WEB_COLOUR_DL        "web.colour_dl"
#define CONFIG_WEB_COLOUR_UL        "web.colour_ul"
#define CONFIG_WEB_ALLOW_REMOTE     "web.allow_remote"
#define CONFIG_WEB_RSS_HOST         "web.rss.host"
#define CONFIG_WEB_RSS_FREQ         "web.rss.freq"
#define CONFIG_WEB_RSS_ITEMS        "web.rss.items"
#define CONFIG_DB_WRITE_INTERVAL    "cap.write_interval"

#define ALLOW_LOCAL_CONNECT_ONLY 0
#define ALLOW_REMOTE_CONNECT 1
#define ALLOW_REMOTE_ADMIN 2
// ----
#define LOG_DEBUG 1
#define LOG_INFO  2
#define LOG_WARN  3
#define LOG_ERR   4
// ----
#define SECS_PER_MIN    60
#define SECS_PER_HOUR   60 * 60
#define MAX_PATH_LEN    256
#define MAC_ADDR_LEN 6
// ----
#define DL_FLAG 1
#define UL_FLAG 2
// ----
#if defined(__APPLE__) || defined(_WIN32)
#define strdupa(s) strcpy(alloca(strlen(s)+1), s)
#endif
// ----
// These are very important, used everywhere
#define TRUE  1
#define FALSE 0

#define SUCCESS 1
#define FAIL    0

typedef unsigned long long BW_INT;

struct Data{
	time_t ts;
	int    dr;
	BW_INT dl;
	BW_INT ul;
	char*  ad;
	char*  hs;
	struct Data* next;
};

struct DateCriteriaPart{
	int isRelative;
	int val1;
	int val2;
	struct DateCriteriaPart* next; // used eg 1-2,3,6-10 is 3 separate DateCriteriaParts
};

struct DateCriteria{ 
	struct DateCriteriaPart* year;
	struct DateCriteriaPart* month;
	struct DateCriteriaPart* day;
	struct DateCriteriaPart* weekday;
	struct DateCriteriaPart* hour;
	struct DateCriteria* next;
};

struct Alert{
    int id;
    char* name;
    int active;
    struct DateCriteria* bound;
    struct DateCriteria* periods;
    int direction;
    BW_INT amount;
    struct Alert* next;
};

#ifndef _WIN32
	typedef int SOCKET;
#endif

// ----
void prepareSql(sqlite3_stmt**, const char*);
sqlite3* openDb();
int isDbOpen();
void prepareDb();
struct Data* runSelect(sqlite3_stmt *stmt);
int runUpdate(sqlite3_stmt* stmt);
void runSelectAndCallback(sqlite3_stmt *stmt, void (*callback)(int, struct Data*), int handle);
int executeSql(const char* sql, int (*callback)(void*, int, char**, char**) );
sqlite3_stmt* getStmt(char*);
void finishedStmt(sqlite3_stmt*);
void beginTrans(int immediate);
void commitTrans();
void rollbackTrans();
void dbVersionCheck();
void setBusyWait(int waitInMs);
const char* getDbError();
void closeDb();
int getConfigInt(const char* key, int quiet);
char* getConfigText(const char* key, int quiet);
int setConfigTextValue(char* key, char* value);
int setConfigIntValue(char* key, int value);
int rmConfigValue(char* key);
int getDbVersion();
// ----
struct DateCriteria* makeDateCriteria(char* yearTxt, char* monthTxt, char* dayTxt, char* weekdayTxt, char* hourTxt);
int isDateCriteriaMatch(struct DateCriteria* criteria, time_t ts);
time_t findFirstMatchingDate(struct DateCriteria* criteria, time_t now);
// ----
struct Data* allocData();
struct Data makeData();
void freeData(struct Data* );
void appendData(struct Data** , struct Data* );
void setAddress(struct Data* data, const char* addr);
void setHost(struct Data* data, const char* host);
// ----
struct Alert* allocAlert();
void freeAlert(struct Alert* alert);
void appendAlert(struct Alert** earlierAlert, struct Alert* newAlert);
void setAlertName(struct Alert* alert, const char* name);
struct DateCriteriaPart* makeDateCriteriaPart(char* txt);
void freeDateCriteriaPart(struct DateCriteriaPart* criteriaPart);
char* dateCriteriaPartToText(struct DateCriteriaPart* part);
void appendDateCriteria(struct DateCriteria** earlierCriteria, struct DateCriteria* newCriteria);
// ----
void doSleep(int interval);
void getDbPath(char* path);
void getLogPath(char* path);
void getWebRootPath(char* path);
#ifdef _WIN32
void logWin32ErrMsg(char* msg, int rc);
#endif
// ----
void setLogToFile(int );
void setLogLevel(int);
void setAppName(const char*);
int isLogDebug();
int isLogInfo();
void logMsg(int level, char* msg, ...);
void vlogMsg(int level, char* msg, va_list argp);
void statusMsg(const char* msg, ...);
void resetStatusMsg();
// ----
void formatAmount(const BW_INT amount, const int binary, const int abbrev, char* txt);
void toTime(char* timeText, time_t ts);
void toDate(char* dateText, time_t ts);
void makeHexString(char* hexString, const char* data, int dataLen);
BW_INT strToBwInt(char* txt, BW_INT defaultValue);
long strToLong(char* txt, long defaultValue);
int strToInt(char* txt, int defaultValue);
char *trim(char *str);
// ----
time_t getTime();
time_t getCurrentLocalYearForTs(time_t ts);
time_t getCurrentLocalMonthForTs(time_t ts);
time_t getCurrentLocalDayForTs(time_t ts);
time_t getNextYearForTs(time_t ts);
time_t getNextLocalYearForTs(time_t ts);
time_t getNextMonthForTs(time_t ts);
time_t getNextLocalMonthForTs(time_t ts);
time_t getNextDayForTs(time_t ts);
time_t getNextLocalDayForTs(time_t ts);
time_t getNextHourForTs(time_t ts);
time_t getNextMinForTs(time_t ts);
time_t addToDate(time_t ts, char unit, int num);
struct tm getLocalTime(time_t t);
void normaliseTm(struct tm* t);

#endif //#ifndef COMMON_H
