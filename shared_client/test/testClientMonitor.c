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
Contains unit tests for the clientMonitor module.
*/

void testMonitorEmptyDb(CuTest *tc) {
 // Check that we behave correctly when the data table is empty
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();

    CuAssertTrue(tc, getMonitorValues(now - 1, NULL, NULL) == NULL);
}

void testMonitorNoDataAfterTs(CuTest *tc) {
 // Check that we behave correctly when the data table contains rows but none meet our criterion
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1, "eth0", 123, 456, "");

    CuAssertTrue(tc, getMonitorValues(now - 1, NULL, NULL) == NULL);
}

void testMonitorDataOnTs(CuTest *tc) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and they all have the same timestamp */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 1, 10, "");
    addDbRow(now - 1, 1, "eth1", 1, 10, "");
    addDbRow(now - 1, 1, "eth2", 1, 10, "");

    struct Data* data = getMonitorValues(now - 1, NULL, NULL);
    checkData(tc, data, now-1, 1, NULL, 3, 30, NULL); // We group data by ts, so expect just 1 result
    CuAssertTrue(tc, data->next == NULL);
}

void testMonitorDataOnAndAfterTs(CuTest *tc) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and have differing timestamps */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 1, 10, "");
    addDbRow(now - 1, 1, "eth1", 1, 10, "");
    addDbRow(now - 1, 1, "eth2", 1, 10, "");

    addDbRow(now - 2, 1, "eth0", 5, 50, "");
    addDbRow(now - 2, 1, "eth1", 5, 50, "");
    addDbRow(now - 2, 1, "eth2", 5, 50, "");

    struct Data* data = getMonitorValues(now - 2, NULL, NULL);
    checkData(tc, data, now-1, 1, NULL, 3, 30, NULL);

    data = data->next;
    checkData(tc, data, now-2, 1, NULL, 15, 150, NULL);

    CuAssertTrue(tc, data->next == NULL);
}

void testMonitorDataOnAndLongAfterTs(CuTest *tc) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and have differing non-consecutive timestamps */
    time_t now  = makeTs("2009-01-01 10:00:00");
    time_t then = makeTs("2008-10-10 11:00:00");

    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 1, 10, "");
    addDbRow(now, 1, "eth1", 1, 10, "");
    addDbRow(now, 1, "eth2", 1, 10, "");

    addDbRow(then + 3600, 3600, "eth0", 5, 50, "");
    addDbRow(then,        3600, "eth1", 6, 60, "");
    addDbRow(then - 3600, 3600, "eth2", 7, 70, "");

    struct Data* data = getMonitorValues(then, NULL, NULL);
    checkData(tc, data, now, 1, NULL, 3, 30, NULL);

    data = data->next;
    checkData(tc, data, then + 3600, 3600, NULL, 5, 50, NULL);

    data = data->next;
    checkData(tc, data, then, 3600, NULL, 6, 60, NULL);

    CuAssertTrue(tc, data->next == NULL);
}

void testMonitorDataForHost(CuTest *tc) {
 /* Check that host selection works correctly. */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1, "eth2",   1,   10, "host1"); // too old
    addDbRow(now - 1, 1, "eth0",   2,   20, "");
    addDbRow(now - 1, 1, "eth1",   4,   40, "host1"); // want this
    addDbRow(now - 1, 1, "eth2",   8,   80, "host1"); // want this
    addDbRow(now - 1, 1, "eth2",  16,  160, "host1"); // want this
    addDbRow(now - 1, 1, "eth2",  32,  320, "host2");
    addDbRow(now + 1, 1, "eth2",  64,  640, "host1"); // want this
    addDbRow(now + 1, 1, "eth2", 128, 1280, "host3");

    struct Data* data = getMonitorValues(now - 1, "host1", NULL);
    checkData(tc, data, now + 1, 1, NULL, 64, 640, NULL);
    data = data->next;
    checkData(tc, data, now - 1, 1, NULL, 28, 280, NULL);

    CuAssertTrue(tc, data->next == NULL);
}

void testMonitorDataForHostAndAdapter(CuTest *tc) {
 /* Check that host/adapter selection works correctly. */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1, "eth2",   1,   10, "host1"); // too old
    addDbRow(now - 1, 1, "eth0",   2,   20, "");
    addDbRow(now - 1, 1, "eth1",   4,   40, "host1"); // wrong adapter
    addDbRow(now - 1, 1, "eth2",   8,   80, "host1"); // want this
    addDbRow(now - 1, 1, "eth2",  16,  160, "host1"); // want this
    addDbRow(now - 1, 1, "eth2",  32,  320, "host2");
    addDbRow(now + 1, 1, "eth2",  64,  640, "host1"); // want this
    addDbRow(now + 1, 1, "eth2", 128, 1280, "host3");

    struct Data* data = getMonitorValues(now - 1, "host1", "eth2");
    checkData(tc, data, now + 1, 1, NULL, 64, 640, NULL);
    data = data->next;
    checkData(tc, data, now - 1, 1, NULL, 24, 240, NULL);

    CuAssertTrue(tc, data->next == NULL);
}

CuSuite* clientMonitorGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testMonitorEmptyDb);
    SUITE_ADD_TEST(suite, testMonitorNoDataAfterTs);
    SUITE_ADD_TEST(suite, testMonitorDataOnTs);
    SUITE_ADD_TEST(suite, testMonitorDataOnAndAfterTs);
    SUITE_ADD_TEST(suite, testMonitorDataOnAndLongAfterTs);
    SUITE_ADD_TEST(suite, testMonitorDataForHost);
    SUITE_ADD_TEST(suite, testMonitorDataForHostAndAdapter);
    return suite;
}
