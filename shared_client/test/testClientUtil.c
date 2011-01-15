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
#include "client.h"
#include "CuTest.h"

/*
Contains unit tests for the clientUtil module.
*/

static void checkTsBounds(CuTest *tc, struct ValueBounds* tsBounds, time_t min, time_t max);
static void checkHostAdapter(CuTest *tc, char* txt, char* host, char* adapter);

void testCalcTsBoundsEmptyDb(CuTest *tc) {
 // Check that we behave correctly when the data table is empty
    emptyDb();
    CuAssertTrue(tc, calcTsBounds(NULL, NULL) == NULL);
    CuAssertTrue(tc, calcTsBounds("host1", NULL) == NULL);
    CuAssertTrue(tc, calcTsBounds("host1", "eth0") == NULL);
}

void testCalcTsBoundsNoMatches(CuTest *tc) {
 // Check that we behave correctly when the data table contains data but no host/adapter matches are found
    emptyDb();

    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, "eth0", 1, 101, "");
    addDbRow(makeTs("2009-05-01 02:00:00"), 3600, "eth1", 2, 102, "host1");

    CuAssertTrue(tc, calcTsBounds("", "eth1") == NULL);
    CuAssertTrue(tc, calcTsBounds("host1", "eth0") == NULL);
}

void testCalcTsBoundsWithMatches(CuTest *tc) {
 // Check that we behave correctly when the data table contains data that matches the host/adapter
    emptyDb();

    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, "eth0", 1, 1, "");
    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, "eth1", 1, 1, "host1");
    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, "eth0", 1, 1, "host2");
    addDbRow(makeTs("2009-05-01 02:00:00"), 3600, "eth0", 1, 1, "");
    addDbRow(makeTs("2009-05-01 02:00:00"), 3600, "eth1", 1, 1, "host1");
    addDbRow(makeTs("2009-05-01 03:00:00"), 3600, "eth1", 1, 1, "");

    checkTsBounds(tc, calcTsBounds("", "eth0"),      makeTs("2009-05-01 01:00:00"), makeTs("2009-05-01 02:00:00"));
    checkTsBounds(tc, calcTsBounds("host1", "eth1"), makeTs("2009-05-01 01:00:00"), makeTs("2009-05-01 02:00:00"));
    checkTsBounds(tc, calcTsBounds("host2", "eth0"), makeTs("2009-05-01 01:00:00"), makeTs("2009-05-01 01:00:00"));
    checkTsBounds(tc, calcTsBounds("", "eth1"),      makeTs("2009-05-01 03:00:00"), makeTs("2009-05-01 03:00:00"));
}

void testGetHostAdapter(CuTest *tc){
    checkHostAdapter(tc, "", "", NULL);
    checkHostAdapter(tc, "host", "host", NULL);
    checkHostAdapter(tc, "host:adapter", "host", "adapter");
}

static void checkTsBounds(CuTest *tc, struct ValueBounds* tsBounds, time_t min, time_t max){
    CuAssertIntEquals(tc, min, tsBounds->min);
    CuAssertIntEquals(tc, max, tsBounds->max);
}

static void checkHostAdapter(CuTest *tc, char* txt, char* host, char* adapter){
    struct HostAdapter* hostAdapter = getHostAdapter(txt);

    CuAssertStrEquals(tc, host, hostAdapter->host);
    CuAssertStrEquals(tc, adapter, hostAdapter->adapter);
}

CuSuite* clientUtilSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testCalcTsBoundsEmptyDb);
    SUITE_ADD_TEST(suite, testCalcTsBoundsNoMatches);
    SUITE_ADD_TEST(suite, testCalcTsBoundsWithMatches);
    SUITE_ADD_TEST(suite, testGetHostAdapter);
    return suite;
}
