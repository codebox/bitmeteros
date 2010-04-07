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

#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include "CuTest.h"
#include "bmws.h"

/*
Contains unit tests for the handleSync module.
*/

void testSyncNoTsParam(CuTest *tc) {
 // The 'ts' parameter is required, so we should get an HTTP error if its missing
    struct Request req = {"GET", "/sync", NULL, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    processSyncRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 500 Bad/missing parameter" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
    , result);
}

void testSyncTsParamOk(CuTest *tc) {
    char ts[20];
    sprintf(ts, "%d", (int)makeTs("2009-11-01 10:00:00"));
    struct NameValuePair param = {"ts", ts, NULL};
    struct Request req = {"GET", "/sync", &param, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    emptyDb();
    addDbRow(makeTs("2009-10-31 10:00:00"), 1, "eth0",  1,  1, NULL);
    addDbRow(makeTs("2009-11-01 10:00:00"), 1, "eth1",  2,  2, NULL);
    addDbRow(makeTs("2009-11-01 10:00:01"), 1, "eth2",  4,  4, "mac");
    addDbRow(makeTs("2009-11-02 00:00:00"), 1, "eth1",  8,  8, NULL);
    addDbRow(makeTs("2009-11-02 00:00:00"), 1, "eth2", 16, 16, NULL);
    addDbRow(makeTs("2009-11-02 01:00:00"), 1, "eth1", 32, 32, NULL);
    addDbRow(makeTs("2010-01-01 00:00:00"), 1, "eth0", 64, 64, "linux");

    int tmpFd = makeTmpFile();
    processSyncRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/vnd.codebox.bitmeter-sync" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "1257120000,1,8,8,eth1" HTTP_EOL
        "1257120000,1,16,16,eth2" HTTP_EOL
        "1257123600,1,32,32,eth1" HTTP_EOL
    , result);
}

CuSuite* handleSyncGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testSyncNoTsParam);
    SUITE_ADD_TEST(suite, testSyncTsParamOk);
    return suite;
}
