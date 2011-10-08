#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include "bmdb.h"
#include "string.h"
#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 

/*
Contains unit tests for the bmdb upgrade module.
*/
int doUpgradeTest(int level);

void testUpgradeToCurrentLevel(void** state){
    addConfigRow(CONFIG_DB_VERSION, "10");

	expect_string(printf_output, msg, "Database is already at level 10, nothing to do."); 
	
    int status = doUpgradeTest(10);
    assert_true(status == FAIL);
    freeStmtList();
}

void testUpgradeToEarlierLevel(void** state){
    addConfigRow(CONFIG_DB_VERSION, "10");
    
    expect_string(printf_output, msg, "Cannot upgrade database to level 9, the database is already at level 10."); 
    
    int status = doUpgradeTest(9);
    assert_true(status == FAIL);
    freeStmtList();
}

void testUpgradeAboveMaxLevel(void** state){
    addConfigRow(CONFIG_DB_VERSION, "1");
    
    expect_string(printf_output, msg, "Cannot upgrade database to level 9, the maximum available upgrade level is 8."); 
    
    int status = doUpgradeTest(MAX_UPGRADE_LEVEL + 1);
    assert_true(status == FAIL);
    freeStmtList();
}

void testUpgradeFrom1To2(void** state){
	executeSql("drop table data2", NULL);
    executeSql("create table data2 (ts,dl,ul,dr,ad);", NULL);
    addConfigRow(CONFIG_DB_VERSION, "1");
    
    expect_string(printf_output, msg, "Database level upgraded to 2."); 
    
    int status = doUpgradeTest(2);
    assert_true(status == SUCCESS);
    assert_int_equal(2,     getDbVersion());
    assert_int_equal(1000,  getConfigInt(CONFIG_WEB_MONITOR_INTERVAL, FALSE));
    assert_int_equal(10000, getConfigInt(CONFIG_WEB_SUMMARY_INTERVAL, FALSE));
    assert_int_equal(10000, getConfigInt(CONFIG_WEB_HISTORY_INTERVAL, FALSE));
    assert_int_equal(0,     getConfigInt(CONFIG_WEB_ALLOW_REMOTE, FALSE));
    
    freeStmtList();
}

void testUpgradeFrom2To3(void** state){
    executeSql("drop table data2;", NULL);
    executeSql("create table data2 (ts,dl,ul,dr,ad);", NULL);
	addConfigRow(CONFIG_DB_VERSION, "2");
	
	assert_true(!tableHasColumn("data2", "hs"));    
    expect_string(printf_output, msg, "Database level upgraded to 3."); 
    
    int status = doUpgradeTest(3);
    assert_true(status == SUCCESS);
    assert_int_equal(3, getDbVersion());
    
    assert_true(tableHasColumn("data2", "hs"));    
    
    freeStmtList();
}

void testUpgradeFrom3To4(void** state){
    executeSql("drop table data2;", NULL);
    executeSql("create table data2 (ts,dl,ul,dr,ad,hs);", NULL);
    addConfigRow(CONFIG_DB_VERSION, "3");
    
    expect_string(printf_output, msg, "Database level upgraded to 4."); 
    
    int status = doUpgradeTest(4);
    assert_true(status == SUCCESS);
    assert_int_equal(4,  getDbVersion());
    
    char* configTxt = getConfigText(CONFIG_WEB_SERVER_NAME, FALSE);
    assert_string_equal("", configTxt);
    free(configTxt);
    
    configTxt = getConfigText("web.colour_dl", FALSE);
    assert_string_equal("#ff0000", configTxt);
    free(configTxt);
    
    configTxt = getConfigText("web.colour_ul", FALSE);
    assert_string_equal("#00ff00", configTxt);
    free(configTxt);
    
    freeStmtList();
}

void testUpgradeFrom4To5(void** state){
    executeSql("drop table data2;", NULL);
    executeSql("create table data2 (ts,dl,ul,dr,ad,hs);", NULL);
	addConfigRow(CONFIG_DB_VERSION, "4");

	expect_string(printf_output, msg, "Database level upgraded to 5."); 

    executeSql("INSERT INTO data2 (ts,dr,ad,dl,ul,hs) VALUES (100, 1, 'eth0',  1,  1, 'host1')", NULL);
    executeSql("INSERT INTO data2 (ts,dr,ad,dl,ul,hs) VALUES (101, 1, 'eth0',  2,  2, '')", NULL);
    executeSql("INSERT INTO data2 (ts,dr,ad,dl,ul)    VALUES (102, 1, 'eth0',  4,  4)", NULL);
    executeSql("INSERT INTO data2 (ts,dr,ad,dl,ul)    VALUES (103, 1, 'eth0',  8,  8)", NULL);
    executeSql("INSERT INTO data2 (ts,dr,ad,dl,ul,hs) VALUES (103, 1, 'eth2', 16, 16, 'host2')", NULL);
    
    assert_int_equal(2, getRowCount("select * from data2 where hs is null"));
    
    int status = doUpgradeTest(5);
    assert_true(status == SUCCESS);
    assert_int_equal(5,  getDbVersion());
    
   	assert_int_equal(0, getRowCount("select * from data2 where hs is null"));
   	
   	freeStmtList();
}

void testUpgradeFrom5To6(void** state){
	executeSql("DROP TABLE alert;", NULL);
	executeSql("DROP TABLE interval;", NULL);
	executeSql("DROP TABLE alert_interval;", NULL);
	addConfigRow(CONFIG_DB_VERSION, "5");
	
	assert_true(tableExists("alert") == FALSE);
	assert_true(tableExists("interval") == FALSE);
	assert_true(tableExists("alert_interval") == FALSE);
	
	expect_string(printf_output, msg, "Database level upgraded to 6."); 
	
    int status = doUpgradeTest(6);
    assert_int_equal(6,  getDbVersion());
    assert_true(status == SUCCESS);
    assert_true(tableExists("alert") == TRUE);
	assert_true(tableExists("interval") == TRUE);
	assert_true(tableExists("alert_interval") == TRUE);
	
	freeStmtList();
}

void testUpgradeFrom6To7(void** state){
	addConfigRow(CONFIG_DB_VERSION, "6");
	expect_string(printf_output, msg, "Database level upgraded to 7."); 
	
    int status = doUpgradeTest(7);
    assert_true(status == SUCCESS);
    assert_int_equal(7,  getDbVersion());
    char* txt = getConfigText(CONFIG_WEB_RSS_HOST, FALSE);
    assert_string_equal("", txt);
    free(txt);
    assert_int_equal(1,  getConfigInt(CONFIG_WEB_RSS_FREQ,  FALSE));
    assert_int_equal(10, getConfigInt(CONFIG_WEB_RSS_ITEMS, FALSE));
    
    freeStmtList();
}

int doUpgradeTest(int level){
    char* txt = malloc(4);
    sprintf(txt, "%d", level);
    char **argv = &txt;
    int result = doUpgrade(stdout, 1, argv);
    free(txt);
    return result;
}

void testConvertAddrValues(void** state){
    executeSql("drop table data2;", NULL);
    executeSql("create table data2 (ts,dl,ul,dr,ad,hs);", NULL);
	
    char binaryAddr1[6] = {0, 1, 2, 3, 4, 5};
    char binaryAddr2[6] = {10, 11, 12, 13, 14, 15};
    char binaryAddr3[6] = {15, 0, 1, 2, 3, 4};
    
    addDbRowBinaryAddress(1, 1, 1, 1, binaryAddr1, 6);
    addDbRowBinaryAddress(2, 1, 3, 5, binaryAddr1, 6);
    addDbRowBinaryAddress(3, 1, 4, 6, binaryAddr1, 6);
    addDbRowBinaryAddress(4, 1, 1, 1, binaryAddr2, 6);
    addDbRowBinaryAddress(5, 1, 1, 1, binaryAddr3, 6);
    addDbRowBinaryAddress(6, 1, 1, 1, binaryAddr2, 6);
    convertAddrValues();
    
	assert_int_equal(1, getRowCount("select * from data2 where ts=1 and ad='000102030405'"));
	assert_int_equal(1, getRowCount("select * from data2 where ts=2 and ad='000102030405'"));
	assert_int_equal(1, getRowCount("select * from data2 where ts=3 and ad='000102030405'"));
	assert_int_equal(1, getRowCount("select * from data2 where ts=4 and ad='0A0B0C0D0E0F'"));
	assert_int_equal(1, getRowCount("select * from data2 where ts=5 and ad='0F0001020304'"));
	assert_int_equal(1, getRowCount("select * from data2 where ts=6 and ad='0A0B0C0D0E0F'"));
	
    freeStmtList();
}

