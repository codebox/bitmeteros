#ifndef BMCLIENT_H
#define BMCLIENT_H

#include "common.h"
#include "client.h"

#define CLIENT_NAME   "BitMeterOS Command Line Client"

// This is all for processing the command-line options...
#define PREF_NOT_SET    0
// ----
#define OPT_HELP 'h'
// ----
#define OPT_VERSION 'v'
// ----
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
// ----
#define OPT_DUMP_FORMAT 'o'

#define PREF_DUMP_FORMAT_CSV         1
#define PREF_DUMP_FORMAT_FIXED_WIDTH 2

#define ARG_DUMP_FORMAT_CSV_SHORT         "c"
#define ARG_DUMP_FORMAT_CSV_LONG          "csv"
#define ARG_DUMP_FORMAT_FIXED_WIDTH_SHORT "f"
#define ARG_DUMP_FORMAT_FIXED_WIDTH_LONG  "fixed"
// ----
#define OPT_UNITS 'u'

#define PREF_UNITS_BYTES  UNITS_BYTES
#define PREF_UNITS_ABBREV UNITS_ABBREV
#define PREF_UNITS_FULL   UNITS_FULL

#define ARG_UNITS_BYTES  "b"
#define ARG_UNITS_ABBREV "a"
#define ARG_UNITS_FULL   "f"
// ----
#define OPT_FILTER 'f'
// ----
#define OPT_RANGE 'r'
// ----
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
// ----
#define OPT_BAR_CHARS  'w'
// ----
#define OPT_MAX_AMOUNT 'x'
// ----
#define OPT_MONITOR_TYPE 't'

#define ARG_MONITOR_TYPE_NUMS "n"
#define ARG_MONITOR_TYPE_BAR  "b"

#define PREF_MONITOR_TYPE_NUMS 1
#define PREF_MONITOR_TYPE_BAR  2
// ----
#define DEFAULT_BAR_CHARS  69
#define DEFAULT_MAX_AMOUNT 100000
// ----
#define ERR_OPT_NO_ARGS          "No arguments supplied."
#define ERR_OPT_BAD_MONITOR_TYPE "Unrecognised monitor type argument"
#define ERR_OPT_BAD_DIRECTION    "Unrecognised direction argument"
#define ERR_OPT_BAD_WIDTH        "Invalid -w argument, must be a number > 0"
#define ERR_OPT_BAD_MAX          "Invalid -x argument, must be a number > 0"
#define ERR_OPT_BAD_RANGE        "Invalid range argument, check the Help for acceptable range formats"
#define ERR_OPT_BAD_GROUP        "Unrecognised group type"
#define ERR_OPT_BAD_UNIT         "Unrecognised unit type"
#define ERR_OPT_BAD_DUMP_FORMAT  "Unrecognised dump format"
#define ERR_OPT_BAD_MODE         "Unrecognised mode"
#define ERR_OPT_BAD_ADAPTER      "Bad adapter value"
#define ERR_WTF                  "BitMeter did not understand. "
// ----
#define SHOW_HELP "Use the '-h' option to display help.\n"
// ----
#define OPT_HOST 'a'
// ----
struct Prefs{
    unsigned int mode;
    unsigned int dumpFormat;
    unsigned int units;
    unsigned int help;
    unsigned int version;
    unsigned int rangeFrom;
    unsigned int rangeTo;
    unsigned int group;
    unsigned int barChars;
    unsigned int maxAmount;
    unsigned int monitorType;
    char* filter;
    char* errorMsg;
};
// ----
int parseArgs(int argc, char **argv, struct Prefs*);
void doDump();
void doMonitor();
void doBmClientQuery();
void doSummary();
void doHelp();
void doBmClientVersion();

#ifdef UNIT_TESTING 
    #define DO_HELP mockDoHelp
    #define DO_VERSION mockDoBmClientVersion
    #define DO_DUMP mockDoDump
    #define DO_MONITOR mockDoMonitor
    #define DO_SUMMARY mockDoSummary
    #define DO_QUERY mockDoQuery
    #define PARSE_ARGS mockParseArgs
#else
    #define DO_HELP doHelp
    #define DO_VERSION doBmClientVersion    
    #define DO_DUMP doDump  
    #define DO_MONITOR doMonitor    
    #define DO_SUMMARY doSummary
    #define DO_QUERY doBmClientQuery
    #define PARSE_ARGS parseArgs
#endif
#endif
