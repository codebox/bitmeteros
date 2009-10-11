#ifndef COMMON_H
#define COMMON_H

#include <sqlite3.h>
#include <stddef.h>
#include <time.h>

#define VERSION "0.1.5"
#define DB_NAME      "bitmeter.db"
#define LOG_NAME     "bitmeter.log"
#define OUT_DIR      "BitMeterOS"
#define IN_MEMORY_DB ":memory:"
#define ENV_DB       "BITMETER_DB"
#define ENV_LOG      "BITMETER_LOG"

#define LOG_INFO 1
#define LOG_WARN 2
#define LOG_ERR  3

#define SECS_PER_MIN    60
#define SECS_PER_HOUR   60 * 60
#define MAX_PATH_LEN    256

#define TRUE 1
#define FALSE 0

#define SUCCESS 1
#define FAIL 0

typedef unsigned long long BW_INT;

struct Data{
	time_t ts;
	BW_INT dl;
	BW_INT ul;
	int    dr;
	char*  ad;
	struct Data* next;
};

void prepareSql(sqlite3_stmt**, const char*);
sqlite3* openDb();
void executeSql(const char*, int (*callback)(void*, int, char**, char**) );
struct Data* runSelect(sqlite3_stmt *stmt);
void runSelectAndCallback(sqlite3_stmt *stmt, void (*callback)(struct Data*));
void beginTrans();
void commitTrans();
void rollbackTrans();
const char* getDbError();
void setDbBusyWait(int);
void closeDb();

struct Data* allocData();
struct Data makeData();
void freeData(struct Data* );
void appendData(struct Data** , struct Data* );

void doSleep(int interval);
void getDbPath(char* path);
void getLogPath(char* path);
void setLogToFile(int );
void setLogLevel(int);
void logMsg(int level, char* msg, ...);
void formatAmount(const BW_INT amount, const int binary, const int abbrev, char* txt);
void toTime(char* timeText, time_t ts);
void toDate(char* dateText, time_t ts);

time_t getTime();
int getCurrentYearForTs(time_t ts);
int getCurrentMonthForTs(time_t ts);
int getCurrentDayForTs(time_t ts);
int getNextYearForTs(time_t ts);
int getNextMonthForTs(time_t ts);
int getNextDayForTs(time_t ts);
int getNextHourForTs(time_t ts);
int getNextMinForTs(time_t ts);
int getYearFromTs(time_t ts);
time_t addToDate(time_t ts, char unit, int num);

#ifdef _WIN32
#define EOL "\r\n"
#endif

#ifdef __linux__
#define EOL "\n"
#endif

#ifdef __APPLE__
#define EOL "\n"
#endif

#endif
