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
Contains unit tests for the handleMonitor module.
*/

void testNoTsParam(CuTest *tc) {
 // The 'ts' parameter is required, so we should get an HTTP error if its missing
    struct Request req = {"GET", "/monitor", NULL, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    processMonitorRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 500 Bad/missing parameter" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
    , result);
}

void testTsParamOkNoHostAdapter(CuTest *tc) {
    struct NameValuePair param = {"ts", "120", NULL};
    struct Request req = {"GET", "/monitor", &param, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

 // Expect to get 3 of these back only, the first lies in the future and the fifth is just outside the 120 second limit specified by 'ts'
    emptyDb();
    addDbRow(makeTs("2009-11-08 10:00:01"), 1, "eth0",  1,  1, "");
    addDbRow(makeTs("2009-11-08 10:00:00"), 1, "eth1",  2,  2, "");
    addDbRow(makeTs("2009-11-08 09:59:00"), 1, "eth0",  4,  4, "host1");
    addDbRow(makeTs("2009-11-08 09:58:00"), 1, "eth0",  8,  8, "");
    addDbRow(makeTs("2009-11-08 09:57:59"), 1, "eth2", 16, 16, "");

    int tmpFd = makeTmpFile();
    processMonitorRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "{\"serverTime\" : 1257674400, \"data\" : [{\"dl\": 2,\"ul\": 2,\"ts\": 0,\"dr\": 1},{\"dl\": 4,\"ul\": 4,\"ts\": 60,\"dr\": 1},{\"dl\": 8,\"ul\": 8,\"ts\": 120,\"dr\": 1}]}"
    , result);
}

void testTsParamOkWithHostAdapter(CuTest *tc) {
    struct NameValuePair param1 = {"ts", "120", NULL};
    struct NameValuePair param2 = {"ha", "host1:eth0", &param1};

    struct Request req = {"GET", "/monitor", &param2, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    emptyDb();
    addDbRow(makeTs("2009-11-08 10:00:00"), 1, "eth1",  1,  1, "host1");
    addDbRow(makeTs("2009-11-08 10:00:00"), 1, "eth1",  2,  2, "");
    addDbRow(makeTs("2009-11-08 10:00:00"), 1, "eth0",  4,  4, "host1");
    addDbRow(makeTs("2009-11-08 10:00:00"), 1, "eth0",  8,  8, "host2");
    addDbRow(makeTs("2009-11-08 10:00:00"), 1, "eth1", 16, 16, "host1");

    int tmpFd = makeTmpFile();
    processMonitorRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "{\"serverTime\" : 1257674400, \"data\" : [{\"dl\": 4,\"ul\": 4,\"ts\": 0,\"dr\": 1}]}"
    , result);
}

CuSuite* handleMonitorGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testNoTsParam);
    SUITE_ADD_TEST(suite, testTsParamOkNoHostAdapter);
    SUITE_ADD_TEST(suite, testTsParamOkWithHostAdapter);
    return suite;
}
