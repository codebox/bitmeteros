#define APP_NAME      "BitMeterOS"
#define CLIENT_NAME   "BitMeterOS Command Line Client"
#define EXE_NAME      "bmclient"

#define OPT_HELP 'h'

#define OPT_VERSION 'v'

#define OPT_MODE 'm'

#define PREF_MODE_DUMP    1
#define PREF_MODE_SUMMARY 2
#define PREF_MODE_MONITOR 3
#define PREF_MODE_QUERY   4

#define ARG_MODE_DUMP_SHORT    "d"
#define ARG_MODE_DUMP_LONG     "dump"
#define ARG_MODE_SUMMARY_SHORT "s"
#define ARG_MODE_SUMMARY_LONG  "summary"
#define ARG_MODE_MONITOR_SHORT "m"
#define ARG_MODE_MONITOR_LONG  "monitor"
#define ARG_MODE_QUERY_SHORT   "q"
#define ARG_MODE_QUERY_LONG    "query"


#define OPT_DUMP_FORMAT 'f'

#define PREF_DUMP_FORMAT_CSV         1
#define PREF_DUMP_FORMAT_FIXED_WIDTH 2

#define ARG_DUMP_FORMAT_CSV_SHORT         "c"
#define ARG_DUMP_FORMAT_CSV_LONG          "csv"
#define ARG_DUMP_FORMAT_FIXED_WIDTH_SHORT "f"
#define ARG_DUMP_FORMAT_FIXED_WIDTH_LONG  "fixed"


#define OPT_UNITS 'u'

#define PREF_UNITS_BYTES  1
#define PREF_UNITS_ABBREV 2
#define PREF_UNITS_FULL   3

#define ARG_UNITS_BYTES  "b"
#define ARG_UNITS_ABBREV "a"
#define ARG_UNITS_FULL   "f"

#define OPT_RANGE 'r'

#define OPT_GROUP 'g'

#define PREF_GROUP_HOURS  1
#define PREF_GROUP_DAYS   2
#define PREF_GROUP_MONTHS 3
#define PREF_GROUP_YEARS  4
#define PREF_GROUP_TOTAL  5

#define ARG_GROUP_HOURS  "h"
#define ARG_GROUP_DAYS   "d"
#define ARG_GROUP_MONTHS "m"
#define ARG_GROUP_YEARS  "y"
#define ARG_GROUP_TOTAL  "t"

#define OPT_DIRECTION 'd'

#define PREF_DIRECTION_DL 1
#define PREF_DIRECTION_UL 2

#define ARG_DIRECTION_DL "d"
#define ARG_DIRECTION_UL "u"

#define OPT_BAR_CHARS  'w'
#define OPT_MAX_AMOUNT 'x'

#define OPT_MONITOR_TYPE 't'

#define ARG_MONITOR_TYPE_NUMS "n"
#define ARG_MONITOR_TYPE_BAR  "b"

#define PREF_MONITOR_TYPE_NUMS 1
#define PREF_MONITOR_TYPE_BAR  2

struct Prefs{
	unsigned int mode;
	unsigned int dumpFormat;
	unsigned int units;
	unsigned int help;
	unsigned int version;
	unsigned int rangeFrom;
	unsigned int rangeTo;
	unsigned int group;
	unsigned int direction;
	unsigned int barChars;
	unsigned int maxAmount;
	unsigned int monitorType;
    char* errorMsg;
};

struct BwValues{
	unsigned long long dl;
	unsigned long long ul;
};

struct ValuesBounds{
	unsigned long long min;
	unsigned long long max;
};

int parseArgs(int argc, char **argv);
void doDump();
void doMonitor();
void doQuery();
void doSummary();
void doHelp();
void doVersion();
void formatAmounts(const unsigned long long dl, const unsigned long long ul, char* dlTxt, char *ulTxt, int units);
struct ValuesBounds calcTsBounds();
