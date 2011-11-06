#define _GNU_SOURCE
#include <stdlib.h>
#include "common.h"
#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "client.h"
#include "bmclient.h"

/*
Contains unit tests for the 'dump.c' module.
*/

extern struct Prefs prefs;

static void setupFilters();
static void setupData();

void setUpTestDbForDump(void** state){
    setupTestDb(state);
    setupFilters();
    setupData();
}

static void setupFilters(){
    addFilterRow(1, "Filter 1", "f1", "port 1", NULL);
    addFilterRow(2, "Filter 2", "f2", "port 2", "h1");
    addFilterRow(3, "Filter 3", "f3", "port 3", "host2");
}
static void setupData(){
    addDbRow(1001, 1, 1, 1);    
    addDbRow(1002, 1, 22, 2);
    addDbRow(1003, 1, 333, 3);
    addDbRow(1004, 1, 4444, 1);
}

void testDumpWithDefaults(void** state){
    struct Prefs p = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL}; 
    prefs = p;
    
    expect_string(printf_output, msg, COPYRIGHT);
    
    expect_string(printf_output, msg, "D1003 T1003 T1004 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "4444 "); 
    expect_string(printf_output, msg, "f1\n"); 

    expect_string(printf_output, msg, "D1002 T1002 T1003 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, " 333 "); 
    expect_string(printf_output, msg, "f3\n"); 
    
    expect_string(printf_output, msg, "D1001 T1001 T1002 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "  22 "); 
    expect_string(printf_output, msg, "f2\n"); 

    expect_string(printf_output, msg, "D1000 T1000 T1001 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "f1\n"); 

    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    
    doDump();
    
    assert_true(prefs.units == PREF_UNITS_BYTES);
    assert_true(prefs.dumpFormat == PREF_DUMP_FORMAT_FIXED_WIDTH);
    freeStmtList();
}

void testDumpAbbrevUnits(void** state){
    struct Prefs p = {0, 0, PREF_UNITS_ABBREV, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL}; 
    prefs = p;

    expect_string(printf_output, msg, COPYRIGHT);

    expect_string(printf_output, msg, "D1003 T1003 T1004 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "      4444 "); 
    expect_string(printf_output, msg, "f1\n"); 

    expect_string(printf_output, msg, "D1002 T1002 T1003 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "       333 "); 
    expect_string(printf_output, msg, "f3\n"); 
    
    expect_string(printf_output, msg, "D1001 T1001 T1002 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "        22 "); 
    expect_string(printf_output, msg, "f2\n"); 

    expect_string(printf_output, msg, "D1000 T1000 T1001 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "         1 "); 
    expect_string(printf_output, msg, "f1\n"); 

    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_ABBREV);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_ABBREV);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_ABBREV);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_ABBREV);
    
    doDump();
    freeStmtList();
}

void testDumpFullUnits(void** state){
    struct Prefs p = {0, 0, PREF_UNITS_FULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};   
    prefs = p;
    
    expect_string(printf_output, msg, COPYRIGHT); 

    expect_string(printf_output, msg, "D1003 T1003 T1004 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "             4444 "); 
    expect_string(printf_output, msg, "f1\n"); 

    expect_string(printf_output, msg, "D1002 T1002 T1003 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "              333 "); 
    expect_string(printf_output, msg, "f3\n"); 
    
    expect_string(printf_output, msg, "D1001 T1001 T1002 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "               22 "); 
    expect_string(printf_output, msg, "f2\n"); 

    expect_string(printf_output, msg, "D1000 T1000 T1001 "); 
    expect_string(printf_output, msg, "   1 "); 
    expect_string(printf_output, msg, "                1 "); 
    expect_string(printf_output, msg, "f1\n"); 

    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_FULL);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_FULL);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_FULL);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_FULL);
    
    doDump();
    freeStmtList();
}
    
void testDumpAsCsv(void** state){
    struct Prefs p = {0, PREF_DUMP_FORMAT_CSV, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};  
    prefs = p;

    expect_string(printf_output, msg, "D1003,T1003,T1004,1,4444,f1\n"); 
    expect_string(printf_output, msg, "D1002,T1002,T1003,1,333,f3\n"); 
    expect_string(printf_output, msg, "D1001,T1001,T1002,1,22,f2\n"); 
    expect_string(printf_output, msg, "D1000,T1000,T1001,1,1,f1\n"); 
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    expect_value(mockFormatAmountByUnits, units, PREF_UNITS_BYTES);
    
    doDump();
    freeStmtList();
}
