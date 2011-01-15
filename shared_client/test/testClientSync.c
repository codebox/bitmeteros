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
Contains unit tests for the clientSync module.
*/

void testSyncEmptyDb(CuTest *tc) {
 // Check that we behave correctly when the data table is empty
    emptyDb();
    CuAssertTrue(tc, getSyncValues(1) == NULL);
}

void testSyncNoMatchingData(CuTest *tc) {
 // Check that we behave correctly when the data table contains rows but none meet our criterion
    emptyDb();
    addDbRow(0, 1, "eth0", 123, 456, "other host");
    addDbRow(1, 1, "eth0", 123, 456, "");
    addDbRow(3, 1, "eth0", 123, 456, "other host");

    CuAssertTrue(tc, getSyncValues(1) == NULL);
}

void testSyncDataOnAndAfterTs(CuTest *tc) {
 /* Check that we behave correctly when the data table contains rows that meet our
    criterion, and have differing timestamps */
    emptyDb();
    addDbRow(9,  1, "eth0", 1, 10, "");
    addDbRow(10, 1, "eth0", 1, 10, "");
    addDbRow(10, 1, "eth0", 2, 11, "");
    addDbRow(11, 1, "eth1", 3, 12, "");
    addDbRow(11, 1, "eth1", 4, 13, "other host");

    struct Data* data = getSyncValues(9);
    checkData(tc, data, 10, 1, "eth0", 1, 10, NULL);

    data = data->next;
    checkData(tc, data, 10, 1, "eth0", 2, 11, NULL);

    data = data->next;
    checkData(tc, data, 11, 1, "eth1", 3, 12, NULL);

    CuAssertTrue(tc, data->next == NULL);
}

CuSuite* clientSyncSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testSyncEmptyDb);
    SUITE_ADD_TEST(suite, testSyncNoMatchingData);
    SUITE_ADD_TEST(suite, testSyncDataOnAndAfterTs);
    return suite;
}
