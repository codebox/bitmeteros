/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include "bmdb.h"
#include "string.h"
#include "test.h"
#include "CuTest.h"

/*
Contains unit tests for the bmdb upgrade module.
*/
static int doUpgradeTest(int level);

static void testUpgradeToCurrentLevel(CuTest *tc){
    executeSql("delete from config;", NULL);
    addConfigRow(CONFIG_DB_VERSION, "10");

    int status = doUpgradeTest(10);
    CuAssertTrue(tc, status == FAIL);
    populateConfigTable();
}

static void testUpgradeToEarlierLevel(CuTest *tc){
    executeSql("delete from config;", NULL);
    addConfigRow(CONFIG_DB_VERSION, "10");

    int status = doUpgradeTest(9);
    CuAssertTrue(tc, status == FAIL);
    populateConfigTable();
}

static void testUpgradeAboveMaxLevel(CuTest *tc){
    executeSql("delete from config;", NULL);
    addConfigRow(CONFIG_DB_VERSION, "1");

    int status = doUpgradeTest(MAX_UPGRADE_LEVEL + 1);
    CuAssertTrue(tc, status == FAIL);
    populateConfigTable();
}

static void testUpgradeFrom1To2(CuTest *tc){
    executeSql("drop table data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad);", NULL);
    executeSql("delete from config;", NULL);

    int status = doUpgradeTest(2);
    CuAssertTrue(tc, status == SUCCESS);
    CuAssertIntEquals(tc, 2,     getDbVersion());
    CuAssertIntEquals(tc, 1000,  getConfigInt(CONFIG_WEB_MONITOR_INTERVAL, FALSE));
    CuAssertIntEquals(tc, 10000, getConfigInt(CONFIG_WEB_SUMMARY_INTERVAL, FALSE));
    CuAssertIntEquals(tc, 10000, getConfigInt(CONFIG_WEB_HISTORY_INTERVAL, FALSE));
    CuAssertIntEquals(tc, 0,     getConfigInt(CONFIG_WEB_ALLOW_REMOTE, FALSE));

    executeSql("drop table data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad,hs);", NULL);
    populateConfigTable();
}

static void testUpgradeFrom2To3(CuTest *tc){
    executeSql("drop table data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad);", NULL);
    executeSql("delete from config;", NULL);

    int status = executeSql("SELECT hs FROM data", NULL);
    CuAssertTrue(tc, status == FAIL);

    status = doUpgradeTest(3);
    CuAssertTrue(tc, status == SUCCESS);
    CuAssertIntEquals(tc, 3, getDbVersion());

    status = executeSql("SELECT hs FROM data", NULL);
    CuAssertTrue(tc, status == SUCCESS);

    executeSql("drop table data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad,hs);", NULL);
    populateConfigTable();
}

static void testUpgradeFrom3To4(CuTest *tc){
    executeSql("drop table data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad);", NULL);
    executeSql("delete from config;", NULL);

    int status = doUpgradeTest(4);
    CuAssertTrue(tc, status == SUCCESS);
    CuAssertIntEquals(tc, 4,  getDbVersion());
    CuAssertStrEquals(tc, "", getConfigText(CONFIG_WEB_SERVER_NAME, FALSE));
    CuAssertStrEquals(tc, "#ff0000", getConfigText(CONFIG_WEB_COLOUR_DL, FALSE));
    CuAssertStrEquals(tc, "#00ff00", getConfigText(CONFIG_WEB_COLOUR_UL, FALSE));

    executeSql("drop table data;", NULL);
    executeSql("create table data (ts,dl,ul,dr,ad,hs);", NULL);
    populateConfigTable();
}

static void testUpgradeFrom4To5(CuTest *tc){
    executeSql("delete from data;", NULL);

    addDbRow(100, 1, "eth0", 1, 1, "host1");
    addDbRow(101, 1, "eth0", 2, 2, "");
    addDbRow(102, 1, "eth0", 4, 4, NULL);
    addDbRow(103, 1, "eth0", 8, 8, NULL);
    addDbRow(103, 1, "eth2", 16, 16, "host2");

    int status = doUpgradeTest(5);
    CuAssertTrue(tc, status == SUCCESS);
    CuAssertIntEquals(tc, 5,  getDbVersion());

    sqlite3_stmt* stmt;
    prepareSql(&stmt, "SELECT ts AS ts, ad AS ad, dl AS dl, ul AS ul, dr AS dr, hs AS hs FROM data ORDER BY ts ASC");
    struct Data* data = runSelect(stmt);

    checkData(tc, data, 100, 1, "eth0", 1, 1, "host1");
    data = data->next;
    checkData(tc, data, 101, 1, "eth0", 2, 2, "");
    data = data->next;
    checkData(tc, data, 102, 1, "eth0", 4, 4, "");
    data = data->next;
    checkData(tc, data, 103, 1, "eth0", 8, 8, "");
    data = data->next;
    checkData(tc, data, 103, 1, "eth2", 16, 16, "host2");

    CuAssertTrue(tc, data->next == NULL);
}

static void testUpgradeFrom5To6(CuTest *tc){
	executeSql("DROP TABLE alert;", NULL);
	executeSql("DROP TABLE interval;", NULL);
	executeSql("DROP TABLE alert_interval;", NULL);
	
	CuAssertTrue(tc, tableExists("alert") == FALSE);
	CuAssertTrue(tc, tableExists("interval") == FALSE);
	CuAssertTrue(tc, tableExists("alert_interval") == FALSE);
	
    int status = doUpgradeTest(6);
    CuAssertTrue(tc, status == SUCCESS);
    CuAssertTrue(tc, tableExists("alert") == TRUE);
	CuAssertTrue(tc, tableExists("interval") == TRUE);
	CuAssertTrue(tc, tableExists("alert_interval") == TRUE);
}

static int doUpgradeTest(int level){
    char* txt = malloc(4);
    sprintf(txt, "%d", level);
    char **argv = &txt;
    return doUpgrade(stdout, 1, argv);
}

static void testConvertAddrValues(CuTest *tc){
    char binaryAddr1[6] = {0, 1, 2, 3, 4, 5};
    char binaryAddr2[6] = {10, 11, 12, 13, 14, 15};
    char binaryAddr3[6] = {15, 0, 1, 2, 3, 4};

    emptyDb();
    addDbRowBinaryAddress(1, 1, 1, 1, binaryAddr1, 6);
    addDbRowBinaryAddress(2, 1, 3, 5, binaryAddr1, 6);
    addDbRowBinaryAddress(3, 1, 4, 6, binaryAddr1, 6);
    addDbRowBinaryAddress(4, 1, 1, 1, binaryAddr2, 6);
    addDbRowBinaryAddress(5, 1, 1, 1, binaryAddr3, 6);
    addDbRowBinaryAddress(6, 1, 1, 1, binaryAddr2, 6);
    convertAddrValues();

    sqlite3_stmt* stmt;
    prepareSql(&stmt, "SELECT ts AS ts, ad AS ad, dl AS dl, ul AS ul, dr AS dr FROM data ORDER BY ts ASC");
    struct Data* data = runSelect(stmt);
    checkData(tc, data, 1, 1, "000102030405", 1, 1, NULL);

    data = data->next;
    checkData(tc, data, 2, 1, "000102030405", 3, 5, NULL);

    data = data->next;
    checkData(tc, data, 3, 1, "000102030405", 4, 6, NULL);

    data = data->next;
    checkData(tc, data, 4, 1, "0A0B0C0D0E0F", 1, 1, NULL);

    data = data->next;
    checkData(tc, data, 5, 1, "0F0001020304", 1, 1, NULL);

    data = data->next;
    checkData(tc, data, 6, 1, "0A0B0C0D0E0F", 1, 1, NULL);

    CuAssertTrue(tc, data->next == NULL);
}

CuSuite* bmdbUpgradeGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testUpgradeToCurrentLevel);
    SUITE_ADD_TEST(suite, testUpgradeToEarlierLevel);
    SUITE_ADD_TEST(suite, testUpgradeAboveMaxLevel);
    SUITE_ADD_TEST(suite, testUpgradeFrom1To2);
    SUITE_ADD_TEST(suite, testUpgradeFrom2To3);
    SUITE_ADD_TEST(suite, testUpgradeFrom3To4);
    SUITE_ADD_TEST(suite, testUpgradeFrom4To5);
    SUITE_ADD_TEST(suite, testUpgradeFrom5To6);
    SUITE_ADD_TEST(suite, testConvertAddrValues);
    return suite;
}
