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

#include "test.h"
#include "common.h"
#include <string.h>
#include <stdarg.h>
#include "capture.h"
#include "CuTest.h"
#include "test.h"

/*
Contains unit tests for the sql module.
*/

void setup();
static void cbAppendData(struct Data* data);
static int getRowCount();
static struct Data* storedData;
static void checkTableContents(CuTest *tc, int rowCount, ...);
extern sqlite3_stmt *selectAllStmt;

void testUpdateDbNull(CuTest *tc) {
 // Check that nothing is added to the d/b if we pass in a NULL list
    int rowsBefore = getRowCount();
    updateDb(1, 1, NULL);
    int rowsAfter = getRowCount();
    CuAssertIntEquals(tc, rowsBefore, rowsAfter);
}

void testUpdateDbMultiple(CuTest *tc) {
 // Check that the correct number of rows are added when we pass in multiple structs
    int rowsBefore = getRowCount();
    struct Data data3 = { 3, 3, 5, 1, "eth0", NULL, NULL};
    struct Data data2 = { 2, 2, 5, 1, "eth1", NULL, &data3};
    struct Data data1 = { 1, 1, 5, 1, "eth2", NULL, &data2};

    updateDb(1,1,&data1);
    int rowsAfter = getRowCount();
    CuAssertIntEquals(tc, rowsBefore + 3, rowsAfter);
}

void testGetNextCompressTime(CuTest *tc){
 // Check that the next d/b compress interval is calculated correctly, based on the current time
    int now = 1234;
    setTime(now);
    CuAssertIntEquals(tc, now + 3600, getNextCompressTime());
}

void testCompressSec1Adapter(CuTest *tc){
 // Check that database second->minute compression is performed correctly for a single adapter
    int now = 7200;
    setTime(now);
    emptyDb();
    addDbRow(3601, 1, "eth0",  1,  1, NULL);
    addDbRow(3600, 1, "eth0",  2,  2, NULL);
    addDbRow(3599, 1, "eth0",  4,  4, NULL);
    addDbRow(3598, 1, "eth0",  8,  8, NULL);
    addDbRow(3597, 1, "eth0", 16, 16, NULL);
    compressDb();

    struct Data row1 = {3601, 1,   1,  1, "eth0", NULL, NULL};
    struct Data row2 = {3600, 60, 30, 30, "eth0", NULL, NULL};

    checkTableContents(tc, 2, row1, row2);
}

void testCompressSecMultiAdapters(CuTest *tc){
 // Check that database second->minute compression is performed correctly for multiple adapters
    int now = 7200;
    setTime(now);
    emptyDb();
    addDbRow(3601, 1, "eth0",  1,  1, NULL);
    addDbRow(3601, 1, "eth1",  2,  2, NULL);
    addDbRow(3601, 1, "eth2",  4,  4, NULL);
    addDbRow(3600, 1, "eth0",  8,  8, NULL);
    addDbRow(3600, 1, "eth1", 16, 16, NULL);
    addDbRow(3600, 1, "eth2", 32, 32, NULL);
    addDbRow(3599, 1, "eth0", 64, 64, NULL);
    addDbRow(3598, 1, "eth1",128,128, NULL);
    addDbRow(3597, 1, "eth2",256,256, NULL);
    compressDb();

    struct Data row1 = {3601, 1,    1,   1, "eth0", NULL, NULL};
    struct Data row2 = {3601, 1,    2,   2, "eth1", NULL, NULL};
    struct Data row3 = {3601, 1,    4,   4, "eth2", NULL, NULL};
    struct Data row4 = {3600, 60,  72,  72, "eth0", NULL, NULL};
    struct Data row5 = {3600, 60, 144, 144, "eth1", NULL, NULL};
    struct Data row6 = {3600, 60, 288, 288, "eth2", NULL, NULL};

    checkTableContents(tc, 6, row1, row2, row3, row4, row5, row6);
}

void testCompressSecMultiIterations(CuTest *tc){
 /* Check that database second->minute compression is performed correctly for multiple adapters where
    the data is spread out over time such that multiple compressed rows for each
    adapter will result. */
    int now = 7200;
    setTime(now);
    emptyDb();
    addDbRow(3601, 1, "eth0",    1,    1, NULL);
    addDbRow(3601, 1, "eth1",    2,    2, NULL);
    addDbRow(3601, 1, "eth2",    4,    4, NULL);
    addDbRow(3600, 1, "eth0",    8,    8, NULL);
    addDbRow(3600, 1, "eth1",   16,   16, NULL);
    addDbRow(3600, 1, "eth2",   32,   32, NULL);
    addDbRow(3599, 1, "eth0",   64,   64, NULL);
    addDbRow(3598, 1, "eth1",  128,  128, NULL);
    addDbRow(3597, 1, "eth2",  256,  256, NULL);
    addDbRow(3540, 1, "eth0",  512,  512, NULL);
    addDbRow(3540, 1, "eth1", 1024, 1024, NULL);
    addDbRow(3540, 1, "eth2", 2048, 2048, NULL);
    addDbRow(3539, 1, "eth0", 4096, 4096, NULL);
    addDbRow(3538, 1, "eth1", 8192, 8192, NULL);
    addDbRow(3537, 1, "eth2",16384,16384, NULL);
    compressDb();

    struct Data row1 = {3601, 1,     1,    1, "eth0", NULL, NULL};
    struct Data row2 = {3601, 1,     2,    2, "eth1", NULL, NULL};
    struct Data row3 = {3601, 1,     4,    4, "eth2", NULL, NULL};
    struct Data row4 = {3600, 60,   72,   72, "eth0", NULL, NULL};
    struct Data row5 = {3600, 60,  144,  144, "eth1", NULL, NULL};
    struct Data row6 = {3600, 60,  288,  288, "eth2", NULL, NULL};
    struct Data row7 = {3540, 60, 4608, 4608, "eth0", NULL, NULL};
    struct Data row8 = {3540, 60, 9216, 9216, "eth1", NULL, NULL};
    struct Data row9 = {3540, 60,18432,18432, "eth2", NULL, NULL};

    checkTableContents(tc, 9, row1, row2, row3, row4, row5, row6, row7, row8, row9);
}

void testCompressMin1Adapter(CuTest *tc){
 // Check that database minute->hour compression is performed correctly for a single adapter
    int now = 86400 + 3600;
    setTime(now);
    emptyDb();
    addDbRow(3601, 60, "eth0",  1,  1, NULL);
    addDbRow(3600, 60, "eth0",  2,  2, NULL);
    addDbRow(3599, 60, "eth0",  4,  4, NULL);
    addDbRow(3598, 60, "eth0",  8,  8, NULL);
    addDbRow(3597, 60, "eth0", 16, 16, NULL);
    compressDb();

    struct Data row1 = {3601,   60,   1,  1, "eth0", NULL, NULL};
    struct Data row2 = {3600, 3600,  30, 30, "eth0", NULL, NULL};

    checkTableContents(tc, 2, row1, row2);
}

void testCompressMinMultiAdapters(CuTest *tc){
 // Check that database minute->hour compression is performed correctly for multiple adapters
    int now = 86400 + 3600;
    setTime(now);
    emptyDb();
    addDbRow(3601, 60, "eth0",  1,  1, NULL);
    addDbRow(3601, 60, "eth1",  2,  2, NULL);
    addDbRow(3601, 60, "eth2",  4,  4, NULL);
    addDbRow(3600, 60, "eth0",  8,  8, NULL);
    addDbRow(3600, 60, "eth1", 16, 16, NULL);
    addDbRow(3600, 60, "eth2", 32, 32, NULL);
    addDbRow(3599, 60, "eth0", 64, 64, NULL);
    addDbRow(3598, 60, "eth1",128,128, NULL);
    addDbRow(3597, 60, "eth2",256,256, NULL);
    compressDb();

    struct Data row1 = {3601,   60,   1,   1, "eth0", NULL, NULL};
    struct Data row2 = {3601,   60,   2,   2, "eth1", NULL, NULL};
    struct Data row3 = {3601,   60,   4,   4, "eth2", NULL, NULL};
    struct Data row4 = {3600, 3600,  72,  72, "eth0", NULL, NULL};
    struct Data row5 = {3600, 3600, 144, 144, "eth1", NULL, NULL};
    struct Data row6 = {3600, 3600, 288, 288, "eth2", NULL, NULL};

    checkTableContents(tc, 6, row1, row2, row3, row4, row5, row6);
}

static void checkTableContents(CuTest *tc, int rowCount, ...){
 // Helper function for checking the contents of the database
    va_list ap;
    va_start(ap,rowCount);
    storedData = NULL;
    runSelectAndCallback(selectAllStmt, &cbAppendData);
	sqlite3_reset(selectAllStmt);

    struct Data expected;
    struct Data* pStored   = storedData;

    int i;
    for(i=0; i<rowCount; i++){
        expected = va_arg(ap, struct Data);
        CuAssertTrue(tc, pStored != NULL);
        CuAssertIntEquals(tc, expected.ts, pStored->ts);
        CuAssertIntEquals(tc, expected.dr, pStored->dr);
        CuAssertIntEquals(tc, expected.dl, pStored->dl);
        CuAssertIntEquals(tc, expected.ul, pStored->ul);
        CuAssertStrEquals(tc, expected.ad, pStored->ad);
        CuAssertStrEquals(tc, expected.hs, pStored->hs);

        pStored   = pStored->next;
    }
    va_end(ap);

    CuAssertTrue(tc, pStored == NULL);
}

CuSuite* sqlGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testUpdateDbNull);
    SUITE_ADD_TEST(suite, testUpdateDbMultiple);
    SUITE_ADD_TEST(suite, testCompressSec1Adapter);
    SUITE_ADD_TEST(suite, testCompressSecMultiAdapters);
    SUITE_ADD_TEST(suite, testCompressSecMultiIterations);
    SUITE_ADD_TEST(suite, testCompressMin1Adapter);
    SUITE_ADD_TEST(suite, testCompressMinMultiAdapters);
    SUITE_ADD_TEST(suite, testGetNextCompressTime);
    return suite;
}

static void cbAppendData(struct Data* data){
	appendData(&storedData, data);
}

static int getRowCount(){
 // Helper method that executes the row-count SQL and returns the result
    struct Data* data = runSelect(selectAllStmt);
    int c = 0;
    while(data != NULL){
    	c++;
    	data = data->next;
    }
    return c;
}
