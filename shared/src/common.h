#include <sqlite3.h>
#include <stddef.h>

#define VERSION "0.1.4"
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

void prepareSql(sqlite3_stmt**, const char*);
sqlite3* openDb();
void executeSql(const char*, int (*callback)(void*, int, char**, char**) );
void beginTrans();
void commitTrans();
const char* getDbError();
void setDbBusyWait(int);
void closeDb();

void doSleep(int interval);
void getDbPath(char* path);
void getLogPath(char* path);
void setLogToFile(int );
void setLogLevel(int);
void logMsg(int level, char* msg, ...);
void formatAmount(const unsigned long long amount, const int binary, const int abbrev, char* txt);
void toTime(char* timeText, int ts);
void toDate(char* dateText, int ts);

int getTime();
int getCurrentYearForTs(int ts);
int getCurrentMonthForTs(int ts);
int getCurrentDayForTs(int ts);
int getNextYearForTs(int ts);
int getNextMonthForTs(int ts);
int getNextDayForTs(int ts);
int getNextHourForTs(int ts);
int getNextMinForTs(int ts);
int getYearFromTs(int ts);
int addToDate(int ts, char unit, int num);

#ifdef _WIN32
#define EOL "\r\n"
#endif

#ifdef __linux__
#define EOL "\n"
#endif

#ifdef __APPLE__
#define EOL "\n"
#endif
