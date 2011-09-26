#define _GNU_SOURCE
#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "client.h"
#include "bmclient.h"

/*
Contains unit tests for the 'bmclient.c' module.
*/

struct Prefs _prefs;
static int _parseArgs(int argc, char** argv, struct Prefs* prefs){
	*prefs = _prefs;
	return (int) mock();
}
static void _doHelp(int dummy){
	check_expected(dummy);
}
static void _doVersion(int dummy){
	check_expected(dummy);
}
static void _doSummary(int dummy){
	check_expected(dummy);
}
static void _doMonitor(int dummy){
	check_expected(dummy);
}
static void _doQuery(int dummy){
	check_expected(dummy);
}
static void _doDump(int dummy){
	check_expected(dummy);
}
static void _setLogLevel(int level){
	check_expected(level);
}
static void _dbVersionCheck(int dummy){
	check_expected(dummy);
}
static sqlite3* _openDb(int dummy){
	check_expected(dummy);
}
static void _closeDb(int dummy){
	check_expected(dummy);
}
static void setupMocks(){
	struct BmClientCalls calls = {&_doHelp, &_doVersion, &_doDump, &_doMonitor, 
		&_doSummary, &_doQuery, &_setLogLevel, &_parseArgs, &_openDb, &_closeDb, &_dbVersionCheck};	
	mockBmClientCalls = calls;
}

void testMainHelp(void** state){
	setupMocks();
	struct Prefs p = {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(_parseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	expect_call(_doHelp);
	_main(0, NULL);
}

void testMainVersion(void** state){
	setupMocks();
	struct Prefs p = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(_parseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	expect_call(_doVersion);
	_main(0, NULL);
}

void testMainFailWithError(void** state){
	setupMocks();
	struct Prefs p = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "doh"};
	_prefs = p;
	will_return(_parseArgs, FAIL); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_string(printf_output, msg, "Error: doh\n"); 
	expect_string(printf_output, msg, SHOW_HELP); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	_main(0, NULL);
}

void testMainFailNoError(void** state){
	setupMocks();
	struct Prefs p = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(_parseArgs, FAIL); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_string(printf_output, msg, ERR_WTF); 
	expect_string(printf_output, msg, SHOW_HELP); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	_main(0, NULL);
}

void testMainDoDump(void** state){
	setupMocks();
	struct Prefs p = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(_parseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	expect_call(_openDb);
	expect_call(_dbVersionCheck);
	expect_call(_doDump);
	expect_call(_closeDb);
	_main(0, NULL);
}

void testMainDoSummary(void** state){
	setupMocks();
	struct Prefs p = {PREF_MODE_SUMMARY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(_parseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	expect_call(_openDb);
	expect_call(_dbVersionCheck);
	expect_call(_doSummary);
	expect_call(_closeDb);
	_main(0, NULL);
}

void testMainDoQuery(void** state){
	setupMocks();
	struct Prefs p = {PREF_MODE_QUERY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(_parseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	expect_call(_openDb);
	expect_call(_dbVersionCheck);
	expect_call(_doQuery);
	expect_call(_closeDb);
	_main(0, NULL);
}

void testMainDoMonitor(void** state){
	setupMocks();
	struct Prefs p = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(_parseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(_setLogLevel, level, LOG_INFO); 
	expect_call(_openDb);
	expect_call(_dbVersionCheck);
	expect_call(_doMonitor);
	expect_call(_closeDb);
	_main(0, NULL);
}