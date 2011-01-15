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

#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include "CuTest.h"
#include "bmws.h"

/*
Contains unit tests for the handleFile module.
*/

void testCharSubstitution(CuTest *tc) {
	FILE* inFile = tmpfile();
    fprintf(inFile, "val1=<!--[v1]--> val2=<!--[v2]--> val1=<!--[v1]-->");
    rewind(inFile);

	int outFile = makeTmpFile();    
    
    struct NameValuePair p1 = {"v1", "1", NULL};
    struct NameValuePair p2 = {"v2", "2", &p1};
    
    doSubs(outFile, inFile, &p2);
    
    char* outTxt = readTmpFile();
    
    CuAssertStrEquals(tc, "val1=1 val2=2 val1=1", outTxt);
}


CuSuite* handleFileGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testCharSubstitution);
    return suite;
}
