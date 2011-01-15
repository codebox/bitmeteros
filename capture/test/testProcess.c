/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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

#include "test.h"
#include "common.h"
#include <string.h>
#include "capture.h"
#include "CuTest.h"

/*
Contains unit tests for the process module.
*/

void testExtractDiffsNull(CuTest *tc){
 // Check that the extractDiffs function handles NULLs correctly
    struct Data* diffs;
	
    diffs = extractDiffs(0, NULL, NULL);
    CuAssertPtrEquals(tc, NULL, diffs);

    struct Data dataOk = {0, 5, 1, 2, "eth0", NULL, NULL};
    diffs = extractDiffs(0, &dataOk, NULL);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(0, NULL, &dataOk);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsNoMatches(CuTest *tc){
 // Check that extractDiffs returns NULL when there are no matching addresses in the 2 structs
    struct Data data1 = {0, 5, 1, 2, "eth0", NULL, NULL};
    struct Data data2 = {0, 5, 2, 3, "eth1", NULL, NULL};

    struct Data* diffs;
    diffs = extractDiffs(100, &data1, &data2);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(100, &data2, &data1);
    CuAssertPtrEquals(tc, NULL, diffs);

    struct Data data3 = {0, 5, 3, 4, "eth2", NULL, &data1};
    struct Data data4 = {0, 5, 4, 5, "eth3", NULL, &data2};

    diffs = extractDiffs(101, &data3, &data4);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(101, &data4, &data3);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsNoChange(CuTest *tc){
 /* Check that extractDiffs returns NULL when there are matching addresses, but there has been no
    change in their dl/ul values */
    struct Data data1c = {0, 5, 2, 3, "eth2", NULL, NULL};
    struct Data data1b = {0, 5, 2, 3, "eth1", NULL, &data1c};
    struct Data data1a = {0, 5, 1, 2, "eth0", NULL, &data1b};

    struct Data data2c = {0, 5, 1, 2, "eth0", NULL, NULL};
    struct Data data2b = {0, 5, 2, 3, "eth1", NULL, &data2c};
    struct Data data2a = {0, 5, 1, 2, "eth3", NULL, &data2b};

    struct Data* diffs;
    diffs = extractDiffs(0, &data1a, &data2a);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(0, &data2a, &data1a);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffs1Match(CuTest *tc){
 /* Check that extractDiffs returns the correct values when there is a single match
    in the 2 struct lists */
    struct Data data1c = {0, 5, 2, 3, "eth2", NULL, NULL};
    struct Data data1b = {0, 5, 2, 3, "eth1", NULL, &data1c};
    struct Data data1a = {0, 5, 1, 2, "eth0", NULL, &data1b};

    struct Data data2c = {0, 5, 2, 4, "eth0", NULL, NULL};
    struct Data data2b = {0, 5, 2, 3, "eth3", NULL, &data2c};
    struct Data data2a = {0, 5, 1, 2, "eth4", NULL, &data2b};

	int ts = 100;
    struct Data* diffs;
    diffs = extractDiffs(ts, &data1a, &data2a);
    checkData(tc, diffs, ts, 0, "eth0", 1, 2, NULL);
}

void testExtractDiffsValuesWrap(CuTest *tc){
 /* Check that extractDiffs returns NULL when there is a match in the 2 lists, but
    the values have wrapped around */
    struct Data data1 = {0, 5, 100, 200, "eth0", NULL, NULL};
    struct Data data2 = {0, 5, 1,   2,   "eth0", NULL, NULL};

    struct Data* diffs = extractDiffs(100, &data1, &data2);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsMultiMatch(CuTest *tc){
 /* Check that extractDiffs returns the correct values when there are multiple matches
    in the 2 struct lists */
    struct Data data1d = {0, 5, 2, 4, "eth4", NULL, NULL};
    struct Data data1c = {0, 5, 2, 3, "eth2", NULL, &data1d};
    struct Data data1b = {0, 5, 1, 2, "eth1", NULL, &data1c};
    struct Data data1a = {0, 5, 0, 0, "eth0", NULL, &data1b};

    struct Data data2d = {0, 5, 2, 3, "eth3", NULL, NULL};
    struct Data data2c = {0, 5, 2, 4, "eth2", NULL, &data2d};
    struct Data data2b = {0, 5, 2, 5, "eth1", NULL, &data2c};
    struct Data data2a = {0, 5, 5, 7, "eth0", NULL, &data2b};

	int ts = 100;
    struct Data* diffs;
    diffs = extractDiffs(ts, &data1a, &data2a);
    checkData(tc, diffs, ts, 0, "eth0", 5, 7, NULL);

    diffs = diffs->next;
    checkData(tc, diffs, ts, 0, "eth1", 1, 3, NULL);

    diffs = diffs->next;
    checkData(tc, diffs, ts, 0, "eth2", 0, 1, NULL);

    CuAssertPtrEquals(tc, NULL, diffs->next);
}

void testNoDelayedWrite(CuTest *tc){
	setDbWriteInterval(1);
	emptyDb();
	
 /* Check that extractDiffs returns the correct values when there is a single match
    in the 2 struct lists */
    setTime(1000);
    struct Data data1 = {0, 1, 0, 0, "eth0", NULL, NULL};
	setPrevData(&data1);

    struct Data data2 = {0, 1, 1, 2, "eth0", NULL, NULL};
	setData(&data2);

	int rows;
	
 // One row should have been written
	processCapture();	
	struct Data row1 = {1000, 1, 1, 2, "eth0", NULL, NULL};
    checkTableContents(tc, 1, row1);
    
    setTime(1001);
    struct Data data3 = {0, 1, 2, 4, "eth0", NULL, NULL};
	setData(&data3);

 // Two rows should have been written
	processCapture();	
	struct Data row2 = {1001, 1, 1, 2, "eth0", NULL, NULL};
    checkTableContents(tc, 2, row2, row1);

    setTime(1002);
    struct Data data4 = {0, 1, 3, 6, "eth0", NULL, NULL};
	setData(&data4);

 // Three rows should have been written
	processCapture();	
	struct Data row3 = {1002, 1, 1, 2, "eth0", NULL, NULL};
    checkTableContents(tc, 3, row3, row2, row1);
}

void testDelayedWrite(CuTest *tc){
	setDbWriteInterval(3);
	emptyDb();
	
 /* Check that extractDiffs returns the correct values when there is a single match
    in the 2 struct lists */
    setTime(1000);
    struct Data data1 = {0, 1, 0, 0, "eth0", NULL, NULL};
	setPrevData(&data1);

    struct Data data2 = {0, 1, 1, 2, "eth0", NULL, NULL};
	setData(&data2);

	int rows;
	
 // No rows should have been written yet
	processCapture();	
	CuAssertIntEquals(tc, 0, getRowCount("select * from data"));
	
    setTime(1001);
    struct Data data3 = {0, 1, 2, 4, "eth0", NULL, NULL};
	setData(&data3);

 // No rows should have been written yet
	processCapture();	
	CuAssertIntEquals(tc, 0, getRowCount("select * from data"));

	setTime(1002);
    struct Data data4 = {0, 1, 3, 6, "eth0", NULL, NULL};
	setData(&data4);
	
 // Three rows should have been written
	processCapture();	
	struct Data row1 = {1000, 1, 1, 2, "eth0", NULL, NULL};
	struct Data row2 = {1001, 1, 1, 2, "eth0", NULL, NULL};
	struct Data row3 = {1002, 1, 1, 2, "eth0", NULL, NULL};
    checkTableContents(tc, 3, row3, row2, row1);
}

CuSuite* processGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testExtractDiffsNull);
    SUITE_ADD_TEST(suite, testExtractDiffsNoMatches);
    SUITE_ADD_TEST(suite, testExtractDiffsNoChange);
    SUITE_ADD_TEST(suite, testExtractDiffs1Match);
    SUITE_ADD_TEST(suite, testExtractDiffsValuesWrap);
    SUITE_ADD_TEST(suite, testExtractDiffsMultiMatch);
    SUITE_ADD_TEST(suite, testNoDelayedWrite);
    SUITE_ADD_TEST(suite, testDelayedWrite);
    return suite;
}
