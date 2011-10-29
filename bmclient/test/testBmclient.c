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
extern struct Prefs _prefs;
void testMainHelp(void** state){
	struct Prefs p = {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(mockParseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	expect_call(mockDoHelp);
	_main(0, NULL);
}

void testMainVersion(void** state){
	struct Prefs p = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(mockParseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	expect_call(mockDoBmClientVersion);
	_main(0, NULL);
}

void testMainFailWithError(void** state){
	struct Prefs p = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, "doh"};
	_prefs = p;
	will_return(mockParseArgs, FAIL); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_string(printf_output, msg, "Error: doh\n"); 
	expect_string(printf_output, msg, SHOW_HELP); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	_main(0, NULL);
}

void testMainFailNoError(void** state){
	struct Prefs p = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(mockParseArgs, FAIL); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_string(printf_output, msg, ERR_WTF); 
	expect_string(printf_output, msg, SHOW_HELP); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	_main(0, NULL);
}

void testMainDoDump(void** state){
	struct Prefs p = {PREF_MODE_DUMP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(mockParseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	expect_call(mockOpenDb);
	expect_call(mockDbVersionCheck);
	expect_call(mockDoDump);
	expect_call(mockCloseDb);
	_main(0, NULL);
}

void testMainDoSummary(void** state){
	struct Prefs p = {PREF_MODE_SUMMARY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(mockParseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	expect_call(mockOpenDb);
	expect_call(mockDbVersionCheck);
	expect_call(mockDoSummary);
	expect_call(mockCloseDb);
	_main(0, NULL);
}

void testMainDoQuery(void** state){
	struct Prefs p = {PREF_MODE_QUERY, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(mockParseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	expect_call(mockOpenDb);
	expect_call(mockDbVersionCheck);
	expect_call(mockDoQuery);
	expect_call(mockCloseDb);
	_main(0, NULL);
}

void testMainDoMonitor(void** state){
	struct Prefs p = {PREF_MODE_MONITOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};
	_prefs = p;
	will_return(mockParseArgs, SUCCESS); 
	expect_string(printf_output, msg, COPYRIGHT); 
	expect_value(mockSetLogLevel, level, LOG_INFO); 
	expect_call(mockOpenDb);
	expect_call(mockDbVersionCheck);
	expect_call(mockDoMonitor);
	expect_call(mockCloseDb);
	_main(0, NULL);
}