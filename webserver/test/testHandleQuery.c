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
Contains unit tests for the handleQuery module.
*/

void testMissingParam(CuTest *tc) {
 // The 3 parameters are required, so we should get an HTTP error if they are missing
    struct Request req = {"GET", "/query", NULL, NULL};

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    processQueryRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 500 Bad/missing parameter" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
    , result);
}
void testParamsOk(CuTest *tc) {
    struct NameValuePair fromParam  = {"from", "1257120000", NULL};       // 2009-11-02
    struct NameValuePair toParam    = {"to",   "1257292800", &fromParam}; // 2009-11-04
    struct NameValuePair groupParam = {"group", "5", &toParam};           // Total
    struct Request req = {"GET", "/query", &groupParam, NULL};

 // The query range covers the second, third and fourth rows only
    emptyDb();
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600, NULL,  1,  1, "");
    addDbRow(makeTs("2009-11-02 12:00:00"), 3600, NULL,  2,  2, ""); // Match
    addDbRow(makeTs("2009-11-03 12:00:00"), 3600, NULL,  4,  4, ""); // Match
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600, NULL,  8,  8, ""); // Match
    addDbRow(makeTs("2009-11-05 12:00:00"), 3600, NULL, 16, 16, "");

    int tmpFd = makeTmpFile();
    processQueryRequest(tmpFd, &req);

    char* result = readTmpFile();

    // The 'ts' value = 2009-11-05 00:00:00, ie the end of the date range covered by the query
    // The 'dr' value = 3 * 24 * 3600, ie the number of seconds in 3 days
    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "[{\"dl\": 14,\"ul\": 14,\"ts\": 1257379200,\"dr\": 259200}]"
    , result);
}

void testGroupByDay(CuTest *tc) {
    struct NameValuePair fromParam  = {"from", "0", NULL};
    struct NameValuePair toParam    = {"to",   "1258281927", &fromParam};
    struct NameValuePair groupParam = {"group", "2", &toParam};
    struct Request req = {"GET", "/query", &groupParam, NULL};

    emptyDb();
    addDbRow(makeTs("2009-11-01 10:00:00"), 3600, NULL,  1,  1, "");
    addDbRow(makeTs("2009-11-01 11:00:00"), 3600, NULL,  2,  2, "");
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600, NULL,  4,  4, "");
    addDbRow(makeTs("2009-11-02 09:00:00"), 3600, NULL,  8,  8, "");
    addDbRow(makeTs("2009-11-02 23:00:00"), 3600, NULL, 16, 16, "");

    int tmpFd = makeTmpFile();
    processQueryRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "[{\"dl\": 7,\"ul\": 7,\"ts\": 1257120000,\"dr\": 54000},{\"dl\": 24,\"ul\": 24,\"ts\": 1257206400,\"dr\": 86400}]"
    , result);
}

void testParamsOkReversed(CuTest *tc) {
    struct NameValuePair fromParam  = {"from", "1257292800", NULL};       // 2009-11-04
    struct NameValuePair toParam    = {"to",   "1257206400", &fromParam}; // 2009-11-03
    struct NameValuePair groupParam = {"group", "2", &toParam};           // Days
    struct Request req = {"GET", "/query", &groupParam, NULL};

 // The query range covers the third and fourth rows only
    emptyDb();
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600, NULL,  1,  1, "");
    addDbRow(makeTs("2009-11-02 12:00:00"), 3600, NULL,  2,  2, "");
    addDbRow(makeTs("2009-11-03 12:00:00"), 3600, NULL,  4,  4, ""); // Match
    addDbRow(makeTs("2009-11-04 12:00:00"), 3600, NULL,  8,  8, ""); // Match
    addDbRow(makeTs("2009-11-05 12:00:00"), 3600, NULL, 16, 16, "");

    int tmpFd = makeTmpFile();
    processQueryRequest(tmpFd, &req);

    char* result = readTmpFile();

    // The 'ts' values = 2009-11-04 00:00:00 and 2009-11-05 00:00:00, ie the ends of the 2 days
    // The 'dr' value = 24 * 3600, ie the number of seconds in a day
    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "[{\"dl\": 4,\"ul\": 4,\"ts\": 1257292800,\"dr\": 86400},{\"dl\": 8,\"ul\": 8,\"ts\": 1257379200,\"dr\": 86400}]"
    , result);
}

void testQueryHostAdapterParams(CuTest *tc) {
    struct NameValuePair fromParam  = {"from", "0", NULL};
    struct NameValuePair toParam    = {"to",   "1258281927", &fromParam}; // 2009-11-15...
    struct NameValuePair groupParam = {"group", "2", &toParam};
    struct NameValuePair haParam    = {"ha", "local:eth0", &groupParam};
    struct Request req = {"GET", "/query", &haParam, NULL};

    emptyDb();
    addDbRow(makeTs("2009-11-01 10:00:00"), 3600, "eth0",  1,  1, ""); // Match
    addDbRow(makeTs("2009-11-01 11:00:00"), 3600, "eth1",  2,  2, "");
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600, "eth0",  4,  4, "host1");
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600, "eth0",  8,  8, ""); // Match
    addDbRow(makeTs("2009-11-02 09:00:00"), 3600, "eth1", 16, 16, "host1");
    addDbRow(makeTs("2009-11-02 23:00:00"), 3600, "eth0", 32, 32, ""); // Match

    int tmpFd = makeTmpFile();
    processQueryRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "[{\"dl\": 9,\"ul\": 9,\"ts\": 1257120000,\"dr\": 54000},{\"dl\": 32,\"ul\": 32,\"ts\": 1257206400,\"dr\": 86400}]"
    , result);
}

void testGroupByDayCsv(CuTest *tc) {
    struct NameValuePair fromParam  = {"from", "0", NULL};
    struct NameValuePair toParam    = {"to",   "1258281927", &fromParam};
    struct NameValuePair groupParam = {"group", "2", &toParam};
    struct NameValuePair csvParam   = {"csv", "1", &groupParam};
    struct Request req = {"GET", "/query", &csvParam, NULL};

    emptyDb();
    addDbRow(makeTs("2009-11-01 10:00:00"), 3600, NULL,  1,  1, "");
    addDbRow(makeTs("2009-11-01 11:00:00"), 3600, NULL,  2,  2, "");
    addDbRow(makeTs("2009-11-01 12:00:00"), 3600, NULL,  4,  4, "");
    addDbRow(makeTs("2009-11-02 09:00:00"), 3600, NULL,  8,  8, "");
    addDbRow(makeTs("2009-11-02 23:00:00"), 3600, NULL, 16, 16, "");

    int tmpFd = makeTmpFile();
    processQueryRequest(tmpFd, &req);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: text/csv" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL
        "Content-Disposition: attachment;filename=bitmeterOsQuery.csv" HTTP_EOL HTTP_EOL
        "2009-11-01 09:00:00,7,7\n"
		"2009-11-02 00:00:00,24,24\n"
    , result);
}

CuSuite* handleQueryGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testMissingParam);
    SUITE_ADD_TEST(suite, testParamsOk);
    SUITE_ADD_TEST(suite, testGroupByDay);
    SUITE_ADD_TEST(suite, testParamsOkReversed);
    SUITE_ADD_TEST(suite, testQueryHostAdapterParams);
    SUITE_ADD_TEST(suite, testGroupByDayCsv);
    return suite;
}
