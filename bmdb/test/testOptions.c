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
#include "bmdb.h"
#include "string.h"
#include "test.h"
#include "CuTest.h"

/*
Contains unit tests for the options module.
*/
static void checkBmDbPrefs(CuTest *tc, struct BmDbPrefs expectedPrefs, char* cmdLine);
extern int optind;
/*struct BmDbPrefs{
	int help;
	int version;
	int upgradeTo;
	int config;
	int vacuum;
	int executeCmd;
	char* errorMsg;
	char* cmdText;
};*/
static void testDbConfig1(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 0, 0, 0, 0, ERR_OPT_NO_ARGS, NULL};
	checkBmDbPrefs(tc, prefs, "");
}

static void testDbConfig2(CuTest *tc){
	struct BmDbPrefs prefs = {1, 0, 0, 0, 0, 0, NULL, NULL};
	checkBmDbPrefs(tc, prefs, "-h");
}

static void testDbConfig3(CuTest *tc){
	struct BmDbPrefs prefs = {0, 1, 0, 0, 0, 0, NULL, NULL};
	checkBmDbPrefs(tc, prefs, "-v");
}
static void testDbConfig4(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 3, 0, 0, 0, NULL, NULL};
	checkBmDbPrefs(tc, prefs, "-u3");
}
static void testDbConfig5(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 0, 1, 0, 0, NULL, NULL};
	checkBmDbPrefs(tc, prefs, "-c");
}
static void testDbConfig6(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 0, 0, 1, 0, NULL, NULL};
	checkBmDbPrefs(tc, prefs, "-m");
}
static void testDbConfig7(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 0, 0, 0, 0, ERR_BAD_ARG, NULL};
	checkBmDbPrefs(tc, prefs, "-q");
}
static void testDbConfig8(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 0, 0, 0, 0, ERR_BAD_UPGRADE_LEVEL, NULL};
	checkBmDbPrefs(tc, prefs, "-u0");
}
static void testDbConfig9(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 0, 0, 0, 1, NULL, NULL};
	checkBmDbPrefs(tc, prefs, "-x");
}
static void testDbConfig10(CuTest *tc){
	struct BmDbPrefs prefs = {0, 0, 0, 0, 0, 1, NULL, "do_something"};
	checkBmDbPrefs(tc, prefs, "-x do_something");
}

static void checkBmDbPrefs(CuTest *tc, struct BmDbPrefs expectedPrefs, char* cmdLine){
 // Helper function for checking the values in a Prefs structure
	int argc;
    char** argv;

    parseCommandLine(cmdLine, &argv, &argc);

	struct BmDbPrefs actualPrefs = {0, 0, 0, 0, 0, 0, NULL, NULL};
	parseBmDbArgs(argc, argv, &actualPrefs);

	CuAssertIntEquals(tc, expectedPrefs.help,        actualPrefs.help);
	CuAssertIntEquals(tc, expectedPrefs.version,     actualPrefs.version);
	CuAssertIntEquals(tc, expectedPrefs.upgradeTo,   actualPrefs.upgradeTo);
	CuAssertIntEquals(tc, expectedPrefs.config,      actualPrefs.config);
	CuAssertIntEquals(tc, expectedPrefs.vacuum,      actualPrefs.vacuum);
    CuAssertIntEquals(tc, expectedPrefs.executeCmd,  actualPrefs.executeCmd);

	if (expectedPrefs.errorMsg == NULL){
		CuAssertTrue(tc, actualPrefs.errorMsg == NULL);
	} else {
		CuAssertStrEquals(tc, expectedPrefs.errorMsg, actualPrefs.errorMsg);
	}

	if (expectedPrefs.executeCmd == NULL){
		CuAssertTrue(tc, actualPrefs.executeCmd == NULL);
	} else {
		CuAssertStrEquals(tc, expectedPrefs.executeCmd, actualPrefs.executeCmd);
	}
}

CuSuite* bmdbOptionsGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testDbConfig1);
    SUITE_ADD_TEST(suite, testDbConfig2);
    SUITE_ADD_TEST(suite, testDbConfig3);
    SUITE_ADD_TEST(suite, testDbConfig4);
    SUITE_ADD_TEST(suite, testDbConfig5);
    SUITE_ADD_TEST(suite, testDbConfig6);
    SUITE_ADD_TEST(suite, testDbConfig7);
    SUITE_ADD_TEST(suite, testDbConfig8);
    SUITE_ADD_TEST(suite, testDbConfig9);
    SUITE_ADD_TEST(suite, testDbConfig10);
    return suite;
}
