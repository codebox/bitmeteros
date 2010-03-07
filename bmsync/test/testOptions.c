/*
 * BitMeterOS v0.3.2
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
 *
 * Build Date: Sun, 07 Mar 2010 14:49:47 +0000
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include "common.h"
#include "string.h"
#include "test.h"
#include "bmsync.h"
#include "CuTest.h"

/*
Contains unit tests for the options module of the bmsync utility.
*/

extern int optind;
static void checkPrefs(CuTest *tc, struct SyncPrefs expectedPrefs, char* cmdLine);

static void testEmptyCmdLine(CuTest *tc){
	struct SyncPrefs prefs = {0, 0, NULL, 0, 0, NULL, ERR_OPT_NO_ARGS};
	checkPrefs(tc, prefs, "");
}

static void testPort(CuTest *tc){
	struct SyncPrefs prefsBad = {0, 0, NULL, 0, 0, NULL, ERR_BAD_PORT};

	checkPrefs(tc, prefsBad, "-p 0 h1");
	checkPrefs(tc, prefsBad, "-p 65536 h1");
	checkPrefs(tc, prefsBad, "-p bork h1");

    char *hosts[1];
    hosts[0] = "h1";
	struct SyncPrefs prefsOk1 = {0, 0, hosts, 1, 1, NULL, NULL};
	checkPrefs(tc, prefsOk1, "-p 1 h1");

	struct SyncPrefs prefsOk2 = {0, 0, hosts, 1, 65535, NULL, NULL};
	checkPrefs(tc, prefsOk2, "-p 65535 h1");

	struct SyncPrefs prefsOk3 = {0, 0, hosts, 1, 80, NULL, NULL};
	checkPrefs(tc, prefsOk3, "-p80 h1");
}

static void testVersion(CuTest *tc){
	struct SyncPrefs prefs = {1, 0, NULL, 0, 0, NULL, NULL};
	checkPrefs(tc, prefs, "-v");
}

static void testHelp(CuTest *tc){
	struct SyncPrefs prefs = {0, 1, NULL, 0, 0, NULL, NULL};
	checkPrefs(tc, prefs, "-h");
}

static void testHost(CuTest *tc){
	char* host1[1];
	host1[0] = strdup("h1");

	struct SyncPrefs prefs1 = {0, 0, host1, 1, 0, NULL, NULL};
	checkPrefs(tc, prefs1, "h1");

	char* host2[3];
	host2[0] = strdup("h1");
	host2[1] = strdup("h2");
	host2[2] = strdup("h3");

	struct SyncPrefs prefs2 = {0, 0, host2, 3, 0, NULL, NULL};
	checkPrefs(tc, prefs2, "h1 h2 h3");
}

static void testAlias(CuTest *tc){
	char* host1[2];
	host1[0] = strdup("h1");
	host1[1] = strdup("h2");

	struct SyncPrefs prefs1 = {0, 0, host1, 0, 0, "alias", ERR_MULTIPLE_HOSTS_ONE_ALIAS};
	checkPrefs(tc, prefs1, "-a alias h1 h2");

	char* host2[1];
	host2[0] = strdup("h1");

	struct SyncPrefs prefs2 = {0, 0, host2, 1, 0, "alias", NULL};
	checkPrefs(tc, prefs2, "-a alias h1");
}

static void testVariousValid(CuTest *tc){
	char* host1[1];
	host1[0] = strdup("myhost");

	struct SyncPrefs prefs1 = {0, 0, host1, 1, 8080, "myalias", NULL};
	checkPrefs(tc, prefs1, "-p 8080 -a myalias myhost");

	char* host2[3];
	host2[0] = strdup("h1");
	host2[1] = strdup("h2");
	host2[2] = strdup("h3");

	struct SyncPrefs prefs2 = {0, 0, host2, 3, 10, NULL, NULL};
	checkPrefs(tc, prefs2, "-p10 h1 h2 h3");
}

static void checkPrefs(CuTest *tc, struct SyncPrefs expectedPrefs, char* cmdLine){
 // Helper function for checking the values in a Prefs structure
	int argc = 1;

	char *cmdLineCopy = strdupa(cmdLine);
	char* match = strtok(cmdLineCopy, " ");
	while(match != NULL){
        argc++;
        match = strtok(NULL, " ");
	}

    char* argv[argc];
    argv[0] = NULL;

    int i=1;
    cmdLineCopy = strdupa(cmdLine);
    match = strtok(cmdLineCopy, " ");
	do{
        argv[i++] = match;
        match = strtok(NULL, " ");
	} while(match != NULL);

	struct SyncPrefs actualPrefs = {0, 0, NULL, 0, 0, NULL, NULL};
	optind = 1; // need to reset this global between each call to getopt()
	parseSyncArgs(argc, argv, &actualPrefs);

	CuAssertIntEquals(tc, expectedPrefs.help,      actualPrefs.help);
	CuAssertIntEquals(tc, expectedPrefs.version,   actualPrefs.version);
	CuAssertIntEquals(tc, expectedPrefs.hostCount, actualPrefs.hostCount);
	CuAssertIntEquals(tc, expectedPrefs.port,      actualPrefs.port);

	for (i=0; i<expectedPrefs.hostCount; i++){
		CuAssertStrEquals(tc, expectedPrefs.hosts[i], actualPrefs.hosts[i]);
	}

	if (expectedPrefs.alias == NULL){
		CuAssertTrue(tc, actualPrefs.alias == NULL);
	} else {
		CuAssertStrEquals(tc, expectedPrefs.alias, actualPrefs.alias);
	}

	if (expectedPrefs.errMsg == NULL){
		CuAssertTrue(tc, actualPrefs.errMsg == NULL);
	} else {
		CuAssertStrEquals(tc, expectedPrefs.errMsg, actualPrefs.errMsg);
	}
}

CuSuite* syncOptionsGetSuite() {
    CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, testEmptyCmdLine);
	SUITE_ADD_TEST(suite, testPort);
	SUITE_ADD_TEST(suite, testVersion);
	SUITE_ADD_TEST(suite, testHelp);
	SUITE_ADD_TEST(suite, testHost);
	SUITE_ADD_TEST(suite, testAlias);
	SUITE_ADD_TEST(suite, testVariousValid);
    return suite;
}
