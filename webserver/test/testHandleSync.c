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
    addDbRow(makeTs("2009-10-31 10:00:00"), 1, "eth0",  1,  1, "");
    addDbRow(makeTs("2009-11-01 10:00:00"), 1, "eth1",  2,  2, "");
    addDbRow(makeTs("2009-11-01 10:00:01"), 1, "eth2",  4,  4, "mac");
    addDbRow(makeTs("2009-11-02 00:00:00"), 1, "eth1",  8,  8, "");
    addDbRow(makeTs("2009-11-02 00:00:00"), 1, "eth2", 16, 16, "");
    addDbRow(makeTs("2009-11-02 01:00:00"), 1, "eth1", 32, 32, "");
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
