#define _GNU_SOURCE
#include <stdlib.h>
#include "string.h"
#include <stdarg.h> 
#include <stddef.h> 
#include "getopt.h"
#include "common.h"
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"
#include "client.h"
#include "bmclient.h"

/*
Contains unit tests for the options module.
*/

extern int optind, optreset;
static void checkPrefs(struct Prefs expectedPrefs, char* cmdLine);

static void checkQueryRangeOk(char* cmdLine, time_t from, time_t to){
 // Helper function for range-related tests that we expect to pass
    struct Prefs prefs = {PREF_MODE_QUERY, 0, 0, 0, 0, from, to, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs, cmdLine);
}

static void checkQueryRangeErr(char* cmdLine){
 // Helper function for range-related tests that we expect to fail
    struct Prefs prefs = {PREF_MODE_QUERY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_RANGE};
    checkPrefs(prefs, cmdLine);
}

void testQueryMode(void** state) {
 // Query-mode tests
    struct Prefs prefs1 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_HOURS,  0, 0, 0, NULL, NULL};
    checkPrefs(prefs1, "-mq -r2009 -gh");
    
    struct Prefs prefs2 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_DAYS,   0, 0, 0, NULL, NULL};
    checkPrefs(prefs2, "-mq -r2009 -gd");
    
    struct Prefs prefs3 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_MONTHS, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs3, "-mq -r2009 -gm");
    
    struct Prefs prefs4 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_YEARS,  0, 0, 0, NULL, NULL};
    checkPrefs(prefs4, "-mq -r2009 -gy");
    
    struct Prefs prefs5 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, PREF_GROUP_TOTAL, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs5, "-mq -r2009 -gt");
    
    struct Prefs prefs6 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, NULL, ERR_OPT_BAD_GROUP};
    checkPrefs(prefs6, "-mq -r2009 -gz");
    
    struct Prefs prefs7 = {PREF_MODE_QUERY, 0, PREF_UNITS_BYTES, 0, 0, 1230768000, 1262304000,  0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs7, "-mq -r2009 -ub");
    
    struct Prefs prefs8 = {PREF_MODE_QUERY, 0, PREF_UNITS_ABBREV, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs8, "-mq -r2009 -ua");
    
    struct Prefs prefs9 = {PREF_MODE_QUERY, 0, PREF_UNITS_FULL, 0, 0, 1230768000, 1262304000,   0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs9, "-mq -r2009 -uf");
    
    struct Prefs prefs10 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, NULL, ERR_OPT_BAD_UNIT};
    checkPrefs(prefs10, "-mq -r2009 -uz");
                                                                                             
    struct Prefs prefs11 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs11, "-mq -r2009");
    
    struct Prefs prefs12 = {PREF_MODE_QUERY, 0, 0, 0, 0, 1230768000, 1262304000, 0, 0, 0, 0, "f1", NULL};
    checkPrefs(prefs12, "-mq -r2009 -ff1");
    
    //checkQueryRangeErr("-mq");
    checkQueryRangeErr("-mq -rz");
    checkQueryRangeErr("-mq -r200");
    checkQueryRangeErr("-mq -r200x");
    //checkQueryRangeErr("-mq -r1969123123");
    checkQueryRangeErr("-mq -r20091");
    //checkQueryRangeErr("-mq -r200900");
    //checkQueryRangeErr("-mq -r200913");
    checkQueryRangeErr("-mq -r2009123");
    //checkQueryRangeErr("-mq -r20090100");
    //checkQueryRangeErr("-mq -r20090132");
    //checkQueryRangeErr("-mq -r20090229");
    
    checkQueryRangeErr("-mq -r2009-");
    checkQueryRangeErr("-mq -r2009-x");
    //checkQueryRangeErr("-mq -r2009-1960");
    //checkQueryRangeErr("-mq -r200901-200913");
    //checkQueryRangeErr("-mq -r20090112-20090132");
    //checkQueryRangeErr("-mq -r2009011223-2009011225");
    checkQueryRangeErr("-mq -r2009011222-20090112239");
    
    checkQueryRangeOk("-mq -r1970", 0, 31536000);
    checkQueryRangeOk("-mq -r1970-1970", 0, 31536000);
    checkQueryRangeOk("-mq -r1970-2008", 0, 1230768000);
    checkQueryRangeOk("-mq -r2008-1970", 0, 1230768000);
    
    checkQueryRangeOk("-mq -r197001", 0, 2678400);
    checkQueryRangeOk("-mq -r197001-197001", 0, 2678400);
    checkQueryRangeOk("-mq -r197001-200812", 0, 1230768000);
    checkQueryRangeOk("-mq -r200812-197001", 0, 1230768000);
    
    checkQueryRangeOk("-mq -r19700101", 0, 86400);
    checkQueryRangeOk("-mq -r19700101-19700101", 0, 86400);
    checkQueryRangeOk("-mq -r19700101-20021231", 0, 1041379200);
    checkQueryRangeOk("-mq -r20021231-19700101", 0, 1041379200);
    
    checkQueryRangeOk("-mq -r1970010100", 0, 3600);
    checkQueryRangeOk("-mq -r1970010100-1970010100", 0, 3600);
    checkQueryRangeOk("-mq -r1970010100-2008123123", 0, 1230768000);
    checkQueryRangeOk("-mq -r2008123123-1970010100", 0, 1230768000);
    
    checkQueryRangeOk("-mq -r2008-2001011119", 979239600, 1230768000);
    //checkQueryRangeOk("-mq -r200807-20090101", 1214866800, 1230854400);
    checkQueryRangeOk("-mq -r200807-20090101", 1214866800, 1230854400);
}

void testSummaryMode(void** state) {
 // Summary-mode tests
    struct Prefs prefs1 = {PREF_MODE_SUMMARY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs1, "-ms");

}
void testDumpMode(void** state) {
 // Dump-mode tests
    struct Prefs prefs1 = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs1, "-md");
    
    struct Prefs prefs2 = {PREF_MODE_DUMP, PREF_DUMP_FORMAT_CSV, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs2, "-md -oc");
    
    struct Prefs prefs3 = {PREF_MODE_DUMP, PREF_DUMP_FORMAT_FIXED_WIDTH, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs3, "-md -of");
    
    struct Prefs prefs4 = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_DUMP_FORMAT};
    checkPrefs(prefs4, "-md -oz");
    
    struct Prefs prefs5 = {PREF_MODE_DUMP, 0, PREF_UNITS_BYTES, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs5, "-md -ub");
    
    struct Prefs prefs6 = {PREF_MODE_DUMP, 0, PREF_UNITS_ABBREV, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs6, "-md -ua");
    
    struct Prefs prefs7 = {PREF_MODE_DUMP, 0, PREF_UNITS_FULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs7, "-md -uf");
    
    struct Prefs prefs8 = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_UNIT};
    checkPrefs(prefs8, "-md -uz");
}

void testNoMode(void** state) {
 // Tests without any mode
    struct Prefs prefs1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_NO_ARGS};
    checkPrefs(prefs1, "");
    
    struct Prefs prefs2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_MODE};
    checkPrefs(prefs2, "-mz");
    
    struct Prefs prefs3 = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs3, "-v");
    
    struct Prefs prefs4 = {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs4, "-h");
}

void testMonitorMode(void** state) {
 // Monitor-mode tests
    struct Prefs prefs1 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    checkPrefs(prefs1, "-mm");
    
    struct Prefs prefs2 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, PREF_MONITOR_TYPE_NUMS, NULL, NULL};
    checkPrefs(prefs2, "-mm -tn");
    
    struct Prefs prefs3 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, PREF_MONITOR_TYPE_BAR, NULL, NULL};
    checkPrefs(prefs3, "-mm -tb");
    
    struct Prefs prefs4 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_MONITOR_TYPE};
    checkPrefs(prefs4, "-mm -tz");
    
    struct Prefs prefs8 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, NULL, NULL};
    checkPrefs(prefs8, "-mm -w10");
    
    struct Prefs prefs9 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_WIDTH};
    checkPrefs(prefs9, "-mm -w0");
    
    struct Prefs prefs10 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_WIDTH};
    checkPrefs(prefs10, "-mm -wz");
    
    struct Prefs prefs11 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 1000000, 0, NULL, NULL};
    checkPrefs(prefs11, "-mm -x1000000");
    
    struct Prefs prefs12 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_MAX};
    checkPrefs(prefs12, "-mm -x0");
    
    struct Prefs prefs13 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, ERR_OPT_BAD_MAX};
    checkPrefs(prefs13, "-mm -xz");
    
    struct Prefs prefs15 = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "f1", NULL};
    checkPrefs(prefs15, "-mm -f f1");
   
}

static void checkPrefs(struct Prefs expectedPrefs, char* cmdLine){
    char** argv;
    int argc;
    parseCommandLine(cmdLine, &argv, &argc);
    
 // Helper function for checking the values in a Prefs structure
    struct Prefs actualPrefs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
    
    optind = 1; // need to reset this global between each call to getopt()
    #if HAVE_DECL_OPTRESET
        optreset = 1; // non-portable way to reset state - see getopt.h
    #endif

    parseArgs(argc, argv, &actualPrefs);

    assert_int_equal(expectedPrefs.mode,        actualPrefs.mode);
    assert_int_equal(expectedPrefs.dumpFormat,  actualPrefs.dumpFormat);
    assert_int_equal(expectedPrefs.units,       actualPrefs.units);
    assert_int_equal(expectedPrefs.help,        actualPrefs.help);
    assert_int_equal(expectedPrefs.version,     actualPrefs.version);
    assert_int_equal(expectedPrefs.rangeFrom,   actualPrefs.rangeFrom);
    assert_int_equal(expectedPrefs.rangeTo,     actualPrefs.rangeTo);
    assert_int_equal(expectedPrefs.group,       actualPrefs.group);
    assert_int_equal(expectedPrefs.barChars,    actualPrefs.barChars);
    assert_int_equal(expectedPrefs.maxAmount,   actualPrefs.maxAmount);
    assert_int_equal(expectedPrefs.monitorType, actualPrefs.monitorType);

    if (expectedPrefs.filter == NULL){
        assert_true(actualPrefs.filter == NULL);
    } else {
        assert_string_equal(expectedPrefs.filter, actualPrefs.filter);
        free(actualPrefs.filter);
    }

    if (expectedPrefs.errorMsg == NULL){
        assert_true(actualPrefs.errorMsg == NULL);
    } else {
        assert_string_equal(expectedPrefs.errorMsg, actualPrefs.errorMsg);
        free(actualPrefs.errorMsg);
    }                         
    
    int i=0;
    while(i<argc){
        free(argv[i++]);
    }
    free(argv);
}
