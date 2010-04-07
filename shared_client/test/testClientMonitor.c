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

    CuAssertTrue(tc, getMonitorValues(now - 1) == NULL);
}

void testMonitorNoDataAfterTs(CuTest *tc) {
 // Check that we behave correctly when the data table contains rows but none meet our criterion
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1, "eth0", 123, 456, NULL);

    CuAssertTrue(tc, getMonitorValues(now - 1) == NULL);
}

void testMonitorDataOnTs(CuTest *tc) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and they all have the same timestamp */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 1, 10, NULL);
    addDbRow(now - 1, 1, "eth1", 1, 10, NULL);
    addDbRow(now - 1, 1, "eth2", 1, 10, NULL);

    struct Data* data = getMonitorValues(now - 1);
    checkData(tc, data, now-1, 1, NULL, 3, 30, NULL); // We group data by ts, so expect just 1 result
    CuAssertTrue(tc, data->next == NULL);
}

void testMonitorDataOnAndAfterTs(CuTest *tc) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and have differing timestamps */
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 1, 10, NULL);
    addDbRow(now - 1, 1, "eth1", 1, 10, NULL);
    addDbRow(now - 1, 1, "eth2", 1, 10, NULL);

    addDbRow(now - 2, 1, "eth0", 5, 50, NULL);
    addDbRow(now - 2, 1, "eth1", 5, 50, NULL);
    addDbRow(now - 2, 1, "eth2", 5, 50, NULL);

    struct Data* data = getMonitorValues(now - 2);
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
    addDbRow(now, 1, "eth0", 1, 10, NULL);
    addDbRow(now, 1, "eth1", 1, 10, NULL);
    addDbRow(now, 1, "eth2", 1, 10, NULL);

    addDbRow(then + 3600, 3600, "eth0", 5, 50, NULL);
    addDbRow(then,        3600, "eth1", 6, 60, NULL);
    addDbRow(then - 3600, 3600, "eth2", 7, 70, NULL);

    struct Data* data = getMonitorValues(then);
    checkData(tc, data, now, 1, NULL, 3, 30, NULL);

    data = data->next;
    checkData(tc, data, then + 3600, 3600, NULL, 5, 50, NULL);

    data = data->next;
    checkData(tc, data, then, 3600, NULL, 6, 60, NULL);

    CuAssertTrue(tc, data->next == NULL);
}

CuSuite* clientMonitorGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testMonitorEmptyDb);
    SUITE_ADD_TEST(suite, testMonitorNoDataAfterTs);
    SUITE_ADD_TEST(suite, testMonitorDataOnTs);
    SUITE_ADD_TEST(suite, testMonitorDataOnAndAfterTs);
    SUITE_ADD_TEST(suite, testMonitorDataOnAndLongAfterTs);
    return suite;
}
