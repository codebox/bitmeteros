/*
 * BitMeterOS v0.3.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
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
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

#include "test.h"
#include "common.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "CuTest.h"

/*
Contains unit tests for the 'data' module.
*/

void testAppendData(CuTest *tc){
    struct Data* first = NULL;

 // Check it works with a null initial element
    struct Data data1 = makeData();
	appendData(&first, &data1);
	CuAssertPtrEquals(tc, &data1, first);
    CuAssertTrue(tc, data1.next == NULL);

 // Check we append correctly when initial element is non-null
	struct Data data2 = makeData();
	appendData(&first, &data2);
	CuAssertPtrEquals(tc, &data1, first);
	CuAssertPtrEquals(tc, &data2, data1.next);
	CuAssertTrue(tc, data2.next == NULL);

 // Check we append correctly when using an intermediate list item as the first argument
	struct Data data3 = makeData();
	struct Data* data1Ptr = &data1;
	appendData(&data1Ptr, &data3);
	CuAssertPtrEquals(tc, &data1, first);
	CuAssertPtrEquals(tc, &data2, data1.next);
	CuAssertPtrEquals(tc, &data3, data2.next);
	CuAssertTrue(tc, data3.next == NULL);
}

void testSetAddress(CuTest *tc){
 // Check address is set correctly, and whitespace is trimmed
    struct Data data = makeData();

    CuAssertTrue(tc, data.next == NULL);
    setAddress(&data, "ad1");
    CuAssertStrEquals(tc, "ad1", data.ad);

    setAddress(&data, "  ad2  ");
    CuAssertStrEquals(tc, "ad2", data.ad);

    setAddress(&data, "ad3  ");
    CuAssertStrEquals(tc, "ad3", data.ad);

    setAddress(&data, "  ad4");
    CuAssertStrEquals(tc, "ad4", data.ad);

    setAddress(&data, "");
    CuAssertStrEquals(tc, "", data.ad);

    setAddress(&data, "   ");
    CuAssertStrEquals(tc, "", data.ad);

    setAddress(&data, NULL);
    CuAssertTrue(tc, data.next == NULL);
}

void testSetHost(CuTest *tc){
 // Check hostname is set correctly, and whitespace is trimmed
    struct Data data = makeData();

    CuAssertTrue(tc, data.next == NULL);
    setHost(&data, "host1");
    CuAssertStrEquals(tc, "host1", data.hs);

    setHost(&data, "  host2  ");
    CuAssertStrEquals(tc, "host2", data.hs);

    setHost(&data, "host3  ");
    CuAssertStrEquals(tc, "host3", data.hs);

    setHost(&data, "  host4");
    CuAssertStrEquals(tc, "host4", data.hs);

    setHost(&data, "");
    CuAssertStrEquals(tc, "", data.hs);

    setHost(&data, "   ");
    CuAssertStrEquals(tc, "", data.hs);

    setHost(&data, NULL);
    CuAssertTrue(tc, data.next == NULL);
}

CuSuite* dataGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testAppendData);
    SUITE_ADD_TEST(suite, testSetAddress);
    SUITE_ADD_TEST(suite, testSetHost);
    return suite;
}
