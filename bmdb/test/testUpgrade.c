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
	executeSql("drop table if exists data", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad);", NULL);
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
    executeSql("drop table if exists data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad);", NULL);
	addConfigRow(CONFIG_DB_VERSION, "2");
	
	assert_true(!tableHasColumn("data", "hs"));    
    expect_string(printf_output, msg, "Database level upgraded to 3."); 
    
    int status = doUpgradeTest(3);
    assert_true(status == SUCCESS);
    assert_int_equal(3, getDbVersion());
    
    assert_true(tableHasColumn("data", "hs"));    
    
    freeStmtList();
}

void testUpgradeFrom3To4(void** state){
    executeSql("drop table if exists data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad,hs);", NULL);
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
    executeSql("drop table if exists data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad,hs);", NULL);
	addConfigRow(CONFIG_DB_VERSION, "4");

	expect_string(printf_output, msg, "Database level upgraded to 5."); 

    executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (100, 1, 'eth0',  1,  1, 'host1')", NULL);
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (101, 1, 'eth0',  2,  2, '')", NULL);
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul)    VALUES (102, 1, 'eth0',  4,  4)", NULL);
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul)    VALUES (103, 1, 'eth0',  8,  8)", NULL);
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (103, 1, 'eth2', 16, 16, 'host2')", NULL);
    
    assert_int_equal(2, getRowCount("select * from data where hs is null"));
    
    int status = doUpgradeTest(5);
    assert_true(status == SUCCESS);
    assert_int_equal(5,  getDbVersion());
    
   	assert_int_equal(0, getRowCount("select * from data where hs is null"));
   	
   	freeStmtList();
}

void testUpgradeFrom5To6(void** state){
	executeSql("DROP TABLE IF EXISTS alert;", NULL);
	executeSql("DROP TABLE IF EXISTS interval;", NULL);
	executeSql("DROP TABLE IF EXISTS alert_interval;", NULL);
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
    executeSql("drop table if exists data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad,hs);", NULL);
	
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
    
	assert_int_equal(1, getRowCount("select * from data where ts=1 and ad='000102030405'"));
	assert_int_equal(1, getRowCount("select * from data where ts=2 and ad='000102030405'"));
	assert_int_equal(1, getRowCount("select * from data where ts=3 and ad='000102030405'"));
	assert_int_equal(1, getRowCount("select * from data where ts=4 and ad='0A0B0C0D0E0F'"));
	assert_int_equal(1, getRowCount("select * from data where ts=5 and ad='0F0001020304'"));
	assert_int_equal(1, getRowCount("select * from data where ts=6 and ad='0A0B0C0D0E0F'"));
	
    freeStmtList();
}

void testUpgradeFrom7To8(void** state){
	executeSql("drop table if exists datatmp", NULL);
	executeSql("drop table if exists preUpgrade8Data", NULL);
	executeSql("drop table if exists filter", NULL);
	executeSql("drop table if exists data", NULL);
	executeSql("create table data (ts,dl,ul,dr,ad,hs);", NULL);
	
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (100, 1, 'eth0',  1,   2, 'host1')", NULL);
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (102, 1, 'eth0',  3,   4, '')",      NULL);
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (103, 1, 'eth0',  5,   6, '')",      NULL);
    executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (103, 1, 'eth2',  7,   8, 'host2')", NULL);
	executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (104, 1, 'eth1',  9,  10, '')",      NULL);
	executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (104, 1, 'eth0', 11,  12, '')",      NULL);
	executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (105, 1, 'eth1', 13,  14, '')",      NULL);
	executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (106, 1, 'eth1', 15,  16, 'host1')", NULL);
	executeSql("INSERT INTO data (ts,dr,ad,dl,ul,hs) VALUES (107, 1, 'eth1', 17,  18, 'host1')", NULL);
	
	addConfigRow(CONFIG_DB_VERSION, "7");
	
	executeSql("drop table if exists alert", NULL);
    executeSql("CREATE TABLE alert (id, name, active, bound, direction, amount)", NULL);
	executeSql("insert into alert values (1, 'alert1', 1, 4, 3, 1000)", NULL);
	executeSql("insert into alert values (2, 'alert2', 1, 5, 2, 2000)", NULL);
	executeSql("insert into alert values (3, 'alert3', 1, 6, 1, 3000)", NULL);
			
	expect_string(printf_output, msg, "Database level upgraded to 8."); 
    int status = doUpgradeTest(8);
    assert_true(status == SUCCESS);
    assert_int_equal(8,  getDbVersion());
    
 // This gets created and removed again
   	assert_true(tableExists("datatmp") == FALSE);
   	
 // This gets created and populated with the old data
	assert_true(tableExists("preUpgrade8Data"));
	assert_true(tableHasColumn("preUpgrade8Data", "ts"));
	assert_true(tableHasColumn("preUpgrade8Data", "dr"));
	assert_true(tableHasColumn("preUpgrade8Data", "dl"));
	assert_true(tableHasColumn("preUpgrade8Data", "ul"));
	assert_true(tableHasColumn("preUpgrade8Data", "ad"));
	assert_true(tableHasColumn("preUpgrade8Data", "hs"));
	assert_int_equal(9, getRowCount("SELECT * FROM preUpgrade8Data"));
	
    assert_true(tableExists("filter"));
    struct Filter* filters = readFilters();
    struct Filter* filter = filters;
    
   	checkFilter(filter, 1, "All Downloads",        "dl",  "dst host {adapter}", NULL);
   	filter = filter->next;	
   	checkFilter(filter, 2, "All Uploads",          "ul",  "src host {adapter}", NULL);
   	filter = filter->next;	
   	checkFilter(filter, 3, "Internet Downloads",   "idl", "dst host {adapter} and not (src net {lan})", NULL);
   	filter = filter->next;	
   	checkFilter(filter, 4, "Internet Uploads",     "iul", "src host {adapter} and not (dst net {lan})", NULL);
   	filter = filter->next;	
   	checkFilter(filter, 5, "Downloads from host1", "dl5", "dst host {adapter}", "host1");
   	filter = filter->next;	
   	checkFilter(filter, 6, "Uploads from host1",   "ul6", "src host {adapter}", "host1");
   	filter = filter->next;	
   	checkFilter(filter, 7, "Downloads from host2", "dl7", "dst host {adapter}", "host2");
   	filter = filter->next;	
   	checkFilter(filter, 8, "Uploads from host2",   "ul8", "src host {adapter}", "host2");
   	assert_true(filter->next == NULL);
    
    freeFilters(filters);
    
    assert_true(tableExists("data"));
    assert_true(tableHasColumn("data", "ts"));
    assert_true(tableHasColumn("data", "dr"));
    assert_true(tableHasColumn("data", "vl"));
    assert_true(tableHasColumn("data", "fl"));
    
    struct Data data1  = {100, 1,  2, 6, NULL};
    struct Data data2  = {100, 1,  1, 5, &data1 };
    struct Data data3  = {102, 1,  4, 2, &data2 };
    struct Data data4  = {102, 1,  3, 1, &data3 };
    struct Data data5  = {103, 1,  8, 8, &data4 };
    struct Data data6  = {103, 1,  7, 7, &data5 };
    struct Data data7  = {103, 1,  6, 2, &data6 };
    struct Data data8  = {103, 1,  5, 1, &data7 };
    struct Data data9  = {104, 1, 12, 2, &data8 };
    struct Data data10 = {104, 1, 10, 2, &data9 };
    struct Data data11 = {104, 1, 11, 1, &data10};
    struct Data data12 = {104, 1,  9, 1, &data11};
    struct Data data13 = {105, 1, 14, 2, &data12};
    struct Data data14 = {105, 1, 13, 1, &data13};
    struct Data data15 = {106, 1, 16, 6, &data14};
    struct Data data16 = {106, 1, 15, 5, &data15};
    struct Data data17 = {107, 1, 18, 6, &data16};
    struct Data data18 = {107, 1, 17, 5, &data17};
    
    checkTableContents(&data18);
    
    assert_true(tableExists("alert"));
    assert_true(tableHasColumn("alert", "id"));
    assert_true(tableHasColumn("alert", "name"));
    assert_true(tableHasColumn("alert", "active"));
    assert_true(tableHasColumn("alert", "bound"));
    assert_true(tableHasColumn("alert", "filter"));
    assert_true(tableHasColumn("alert", "amount"));
    assert_true(!tableExists("alerttmp"));
    
    assert_int_equal(1, getRowCount("SELECT * FROM alert WHERE id=2 AND filter=2 AND " 
    		"name='alert2' AND active=1 AND bound=5 AND amount=2000"));
    assert_int_equal(1, getRowCount("SELECT * FROM alert WHERE id=3 AND filter=1 AND " 
    		"name='alert3' AND active=1 AND bound=6 AND amount=3000"));
 // The row with direction=3 should be deleted
    assert_int_equal(0, getRowCount("SELECT * FROM alert WHERE filter=3"));
    
    freeStmtList();
}

