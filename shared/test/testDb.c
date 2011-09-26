#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"
#include "common.h"
#include <string.h>

/*
Contains unit tests for the 'db' module.
*/

void testGetConfigInt(void** state){
    assert_int_equal(-1, getConfigInt("config.int", TRUE));
    addConfigRow("config.int", "7");
    assert_int_equal(7, getConfigInt("config.int", TRUE));
    freeStmtList();
}

void testGetConfigText(void** state){
    assert_true(getConfigText("config.txt", TRUE) == NULL);
    addConfigRow("config.txt", "text");
    char* val = getConfigText("config.txt", TRUE);
    assert_string_equal("text", val);
    free(val);
    freeStmtList();
}

void testSetConfigInt(void** state){
    assert_int_equal(-1, getConfigInt("config.int", TRUE));
    setConfigIntValue("config.int", 7);
    assert_int_equal(7, getConfigInt("config.int", TRUE));
    freeStmtList();
}

void testSetConfigText(void** state){
    assert_true(getConfigText("config.txt", TRUE) == NULL);
    setConfigTextValue("config.txt", "text");
    char* val = getConfigText("config.txt", TRUE);
    assert_string_equal("text", val);
    free(val);
    freeStmtList();
}
void testRmConfigBadValue(void** state){
    assert_true(getConfigText("config.txt", TRUE) == NULL);
    rmConfigValue("config.txt");
    assert_true(getConfigText("config.txt", TRUE) == NULL);	
    freeStmtList();
}
void testRmConfigOkValue(void** state){
    assert_true(getConfigText("config.txt", TRUE) == NULL);
    addConfigRow("config.txt", "text");
    char* val = getConfigText("config.txt", TRUE);
    assert_string_equal("text", val);
    free(val);
    rmConfigValue("config.txt");
    assert_true(getConfigText("config.txt", TRUE) == NULL);	
    freeStmtList();
}
void testReadFiltersEmpty(void** state){
	assert_true(readFilters() == NULL);
	freeStmtList();
}
//void addFilterRow(int id, char* desc, char* name, char* expr, char* host)
void testReadFiltersOk(void** state){
	addFilterRow(1, "desc 1", "f1", "ex1", NULL);
	addFilterRow(2, "desc 2", "f2", "ex2", "host1");
	addFilterRow(3, "desc 3", "f3", "ex3", "host2");
	
	struct Filter* first;
	struct Filter* filter = first = readFilters();
	
	checkFilter(filter, 1, "desc 1", "f1", "ex1", NULL);
	filter = filter->next;
	checkFilter(filter, 2, "desc 2", "f2", "ex2", "host1");
	filter = filter->next;
	checkFilter(filter, 3, "desc 3", "f3", "ex3", "host2");
	
	freeFilters(first);
	freeStmtList();
}

void testGetStmtSingleThreaded(void** state){
	char* sql1 = "select * from data2";
	char* sql2 = "select * from config";
	assert_true(stmtList == NULL);
	
	sqlite3_stmt *stmt1 = getStmtSingleThreaded(sql1);
	assert_true(stmtList != NULL);
	assert_string_equal(sql1, stmtList->sql);
	assert_int_equal(stmt1, stmtList->stmt);
	assert_true(stmtList->next == NULL);
	
	sqlite3_stmt *stmt2 = getStmtSingleThreaded(sql1);
	assert_int_equal(stmt1, stmt2);
	assert_string_equal(sql1, stmtList->sql);
	assert_int_equal(stmt1, stmtList->stmt);
	assert_true(stmtList->next == NULL);
	
	sqlite3_stmt *stmt3 = getStmtSingleThreaded(sql2);
	assert_string_equal(sql2, stmtList->next->sql);
	assert_int_equal(stmt3, stmtList->next->stmt);
	assert_true(stmtList->next->next == NULL);

	sqlite3_stmt *stmt4 = getStmtSingleThreaded(sql1);
	assert_int_equal(stmt1, stmt4);

	sqlite3_stmt *stmt5 = getStmtSingleThreaded(sql2);
	assert_int_equal(stmt3, stmt5);
	
	freeStmtList();
}