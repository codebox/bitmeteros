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
Contains unit tests for the handleSummary module.
*/

void testSummaryNoHosts(CuTest *tc) {
    emptyDb();
    addDbRow(makeTs("2008-11-02 12:00:00"), 3600, NULL,  1,  1, ""); // Last year
    addDbRow(makeTs("2009-10-01 12:00:00"), 3600, NULL,  2,  2, ""); // Earlier this year
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600, NULL,  4,  4, ""); // Earlier this month
    addDbRow(makeTs("2009-11-08 01:00:00"), 3600, NULL,  8,  8, ""); // Today

    struct Request req = {"GET", "/summary", NULL, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    processSummaryRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "{\"today\": {\"dl\": 8,\"ul\": 8,\"ts\": 0,\"dr\": 0}, \"month\": {\"dl\": 12,\"ul\": 12,\"ts\": 0,\"dr\": 0}, \"year\": {\"dl\": 14,\"ul\": 14,\"ts\": 0,\"dr\": 0}, \"total\": {\"dl\": 15,\"ul\": 15,\"ts\": 0,\"dr\": 0}, \"hosts\": [], \"since\": 1225627200}"
    , result);
}

void testSummaryWithHosts(CuTest *tc) {
    emptyDb();
    addDbRow(makeTs("2008-11-02 12:00:00"), 3600, NULL,  1,  1, "server"); // Last year
    addDbRow(makeTs("2009-10-01 12:00:00"), 3600, NULL,  2,  2, ""); // Earlier this year
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600, NULL,  4,  4, "windowsbox"); // Earlier this month
    addDbRow(makeTs("2009-11-08 01:00:00"), 3600, NULL,  8,  8, "windowsbox"); // Today

    struct Request req = {"GET", "/summary", NULL, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    processSummaryRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "{\"today\": {\"dl\": 8,\"ul\": 8,\"ts\": 0,\"dr\": 0}, \"month\": {\"dl\": 12,\"ul\": 12,\"ts\": 0,\"dr\": 0}, \"year\": {\"dl\": 14,\"ul\": 14,\"ts\": 0,\"dr\": 0}, \"total\": {\"dl\": 15,\"ul\": 15,\"ts\": 0,\"dr\": 0}"
        ", \"hosts\": [\"server\", \"windowsbox\"], \"since\": 1225627200}"
    , result);
}

void testSummaryWithHostAdapterParams(CuTest *tc) {
    emptyDb();
    addDbRow(makeTs("2008-11-02 12:00:00"), 3600, "eth0",  1,  1, "host1"); // Last year
    addDbRow(makeTs("2008-11-02 12:00:00"), 3600, "eth0",  1,  1, "host2"); // Last year
    addDbRow(makeTs("2009-10-01 12:00:00"), 3600, "eth0",  2,  2, "host1"); // Earlier this year
    addDbRow(makeTs("2009-10-01 12:00:00"), 3600, "eth0",  2,  2, ""); // Earlier this year
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600, "eth0",  4,  4, "host1"); // Earlier this month
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600, "eth0",  4,  4, ""); // Earlier this month
    addDbRow(makeTs("2009-11-08 01:00:00"), 3600, "eth0",  8,  8, "host1"); // Today
    addDbRow(makeTs("2009-11-08 01:00:00"), 3600, "eth0",  8,  8, ""); // Today

    struct NameValuePair param = {"ha", "host1:eth0", NULL};
    struct Request req = {"GET", "/summary", &param, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    processSummaryRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "{\"today\": {\"dl\": 8,\"ul\": 8,\"ts\": 0,\"dr\": 0}, \"month\": {\"dl\": 12,\"ul\": 12,\"ts\": 0,\"dr\": 0}, \"year\": {\"dl\": 14,\"ul\": 14,\"ts\": 0,\"dr\": 0}, \"total\": {\"dl\": 15,\"ul\": 15,\"ts\": 0,\"dr\": 0}"
        ", \"hosts\": null, \"since\": 1225627200}"
    , result);
}

CuSuite* handleSummaryGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testSummaryNoHosts);
    SUITE_ADD_TEST(suite, testSummaryWithHosts);
    SUITE_ADD_TEST(suite, testSummaryWithHostAdapterParams);
    return suite;
}
