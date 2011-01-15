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
#include <stdlib.h>
#include "CuTest.h"

/*
Contains unit tests for the 'db' module.
*/

void testGetConfigInt(CuTest *tc){
    CuAssertIntEquals(tc, -1, getConfigInt("config.int", TRUE));
    addConfigRow("config.int", "7");
    CuAssertIntEquals(tc, 7, getConfigInt("config.int", TRUE));
}

void testGetConfigText(CuTest *tc){
    CuAssertTrue(tc, getConfigText("config.txt", TRUE) == NULL);
    addConfigRow("config.txt", "text");
    CuAssertStrEquals(tc, "text", getConfigText("config.txt", TRUE));
}

CuSuite* dbGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testGetConfigInt);
    SUITE_ADD_TEST(suite, testGetConfigText);
    return suite;
}
