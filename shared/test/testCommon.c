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
#include <time.h>
#include <stdlib.h>
#include "capture.h"
#include "CuTest.h"

/*
Contains unit tests for the 'common' module.
*/

static void doTestFormatAmount(CuTest *, BW_INT , char* , char* , char *, char* );
static void doTestToTime(CuTest *tc, int ts, char* expected);
static void doTestToDate(CuTest *tc, int ts, char* expected);

void testFormatAmounts(CuTest *tc){
 // Check that various amounts are formatted correctly, using all combinations of binary/decimal and full/abbreviated units
    doTestFormatAmount(tc, 0, "0.00 B ", "0.00 bytes", "0.00 B ", "0.00 bytes");
    doTestFormatAmount(tc, 1, "1.00 B ", "1.00 bytes", "1.00 B ", "1.00 bytes");

    doTestFormatAmount(tc, 999,   "999.00 B ",   "999.00 bytes", "999.00 B ",   "999.00 bytes");
    doTestFormatAmount(tc, 1000, "1000.00 B ",  "1000.00 bytes",   "1.00 kB", "1.00 kilobytes");
    doTestFormatAmount(tc, 1023, "1023.00 B ",  "1023.00 bytes",   "1.02 kB", "1.02 kilobytes");
    doTestFormatAmount(tc, 1024,    "1.00 kB", "1.00 kilobytes",   "1.02 kB", "1.02 kilobytes");

    doTestFormatAmount(tc,  999999,   "976.56 kB",  "976.56 kilobytes", "1000.00 kB",   "1000.00 kilobytes");
    doTestFormatAmount(tc, 1000000 ,  "976.56 kB",  "976.56 kilobytes",    "1.00 MB",      "1.00 megabytes");
    doTestFormatAmount(tc, 1048575,  "1024.00 kB", "1024.00 kilobytes",    "1.05 MB",      "1.05 megabytes");
    doTestFormatAmount(tc, 1048576,     "1.00 MB",    "1.00 megabytes",    "1.05 MB",      "1.05 megabytes");

    doTestFormatAmount(tc,  999999999,   "953.67 MB",  "953.67 megabytes", "1000.00 MB",   "1000.00 megabytes");
    doTestFormatAmount(tc, 1000000000 ,  "953.67 MB",  "953.67 megabytes",    "1.00 GB",      "1.00 gigabytes");
    doTestFormatAmount(tc, 1073741823,  "1024.00 MB", "1024.00 megabytes",    "1.07 GB",      "1.07 gigabytes");
    doTestFormatAmount(tc, 1073741824,     "1.00 GB",    "1.00 gigabytes",    "1.07 GB",      "1.07 gigabytes");

    doTestFormatAmount(tc,  999999999999llu,  "931.32 GB",  "931.32 gigabytes", "1000.00 GB",   "1000.00 gigabytes");
    doTestFormatAmount(tc, 1000000000000llu,  "931.32 GB",  "931.32 gigabytes",    "1.00 TB",      "1.00 terabytes");
    doTestFormatAmount(tc, 1099511627775llu, "1024.00 GB", "1024.00 gigabytes",    "1.10 TB",      "1.10 terabytes");
    doTestFormatAmount(tc, 1099511627776llu,    "1.00 TB",    "1.00 terabytes",    "1.10 TB",      "1.10 terabytes");

    doTestFormatAmount(tc,  999999999999999llu,  "909.49 TB",  "909.49 terabytes", "1000.00 TB",   "1000.00 terabytes");
    doTestFormatAmount(tc, 1000000000000000llu,  "909.49 TB",  "909.49 terabytes",    "1.00 PB",      "1.00 petabytes");
    doTestFormatAmount(tc, 1125899906842623llu, "1024.00 TB", "1024.00 terabytes",    "1.13 PB",      "1.13 petabytes");
    doTestFormatAmount(tc, 1125899906842624llu,    "1.00 PB",    "1.00 petabytes",    "1.13 PB",      "1.13 petabytes");

    doTestFormatAmount(tc,  999999999999999999llu,  "888.18 PB",  "888.18 petabytes", "1000.00 PB",   "1000.00 petabytes");
    doTestFormatAmount(tc, 1000000000000000000llu,  "888.18 PB",  "888.18 petabytes",    "1.00 EB",      "1.00 exabytes");
    doTestFormatAmount(tc, 1152921504606846975llu, "1024.00 PB", "1024.00 petabytes",    "1.15 EB",      "1.15 exabytes");
    doTestFormatAmount(tc, 1152921504606846976llu,    "1.00 EB",     "1.00 exabytes",    "1.15 EB",      "1.15 exabytes");
}

static void doTestFormatAmount(CuTest *tc, BW_INT amount, char* binaryShort, char* binaryLong, char *decimalShort, char* decimalLong){
 // Helper function for checking value formatting using all combinations of binary/decimal and full/abbreviated units
    char txt[24];

    formatAmount(amount, 1, 1, txt);
    CuAssertStrEquals(tc, binaryShort, txt);

    formatAmount(amount, 1, 0, txt);
    CuAssertStrEquals(tc, binaryLong, txt);

    formatAmount(amount, 0, 1, txt);
    CuAssertStrEquals(tc, decimalShort, txt);

    formatAmount(amount, 0, 0, txt);
    CuAssertStrEquals(tc, decimalLong, txt);
}

void testToTime(CuTest *tc){
 // Check that the 'toTime' function correctly extracts the time component from various timestamps
    doTestToTime(tc, 0,  "00:00:00");
    doTestToTime(tc, 1,  "00:00:01");
    doTestToTime(tc, 60, "00:01:00");

    doTestToTime(tc, makeTs("2000-01-01 00:00:00"), "00:00:00");
    doTestToTime(tc, makeTs("2000-01-01 00:00:01"), "00:00:01");
    doTestToTime(tc, makeTs("2000-01-01 00:02:01"), "00:02:01");
    doTestToTime(tc, makeTs("2000-01-01 00:59:59"), "00:59:59");
    doTestToTime(tc, makeTs("2000-01-01 12:12:12"), "12:12:12");
    doTestToTime(tc, makeTs("2000-01-01 23:59:59"), "23:59:59");
}
static void doTestToTime(CuTest *tc, int ts, char* expected){
 // Helper function to check that the 'toTime' function correctly extracts the time component of a timestamp
    char actual[20];
    toTime(actual, ts);
    CuAssertStrEquals(tc, expected, actual);
}

void testToDate(CuTest *tc){
 // Check that the 'toDate' function correctly extracts the date component from various timestamps
    doTestToDate(tc, 0,  "1970-01-01");

    doTestToDate(tc, makeTs("2000-01-01 00:00:00"), "2000-01-01");
    doTestToDate(tc, makeTs("2008-12-31 00:00:00"), "2008-12-31");
    doTestToDate(tc, makeTs("2008-02-29 00:00:00"), "2008-02-29");
}

void testMakeHexString(CuTest *tc){
    char bytes1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    char hex1[36];

    makeHexString(hex1, bytes1, 17);
    CuAssertStrEquals(tc, "000102030405060708090A0B0C0D0E0F10", hex1);

    char bytes2[] = {};
    char hex2[2];

    makeHexString(hex2, bytes2, 0);
    CuAssertStrEquals(tc, "", hex2);

    char bytes3[] = {0, 12, 241, 86, 152, 173};
    char hex3[13];

    makeHexString(hex3, bytes3, 6);
    CuAssertStrEquals(tc, "000CF15698AD", hex3);
}

static void doTestToDate(CuTest *tc, int ts, char* expected){
 // Helper function to check that the 'toDate' function correctly extracts the date component of a timestamp
    char actual[20];
    toDate(actual, ts);
    CuAssertStrEquals(tc, expected, actual);
}

static void testStrToLong(CuTest *tc){
    CuAssertTrue(tc, strToLong("", -1) == -1);
    CuAssertTrue(tc, strToLong("asd", -1) == -1);
    CuAssertTrue(tc, strToLong("12a", -1) == -1);
    CuAssertTrue(tc, strToLong(" 123", -1) == 123);
    CuAssertTrue(tc, strToLong(NULL, -1) == -1);
}

CuSuite* commonGetSuite() {
    putenv("TZ=GMT"); // Need this because the date/time fns use localtime
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testFormatAmounts);
    SUITE_ADD_TEST(suite, testToTime);
    SUITE_ADD_TEST(suite, testToDate);
    SUITE_ADD_TEST(suite, testMakeHexString);
    SUITE_ADD_TEST(suite, testStrToLong);
    return suite;
}
