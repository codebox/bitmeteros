#include "test.h"
#include "common.h"
#include "client.h"
#include "CuTest.h"

void testMonitorEmptyDb(CuTest *tc) {
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();

    CuAssertTrue(tc, getMonitorValues(now - 1) == NULL);
}

void testMonitorNoDataAfterTs(CuTest *tc) {
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 2, 1, "eth0", 123, 456);

    CuAssertTrue(tc, getMonitorValues(now - 1) == NULL);
}

void testMonitorDataOnTs(CuTest *tc) {
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 1, 10);
    addDbRow(now - 1, 1, "eth1", 1, 10);
    addDbRow(now - 1, 1, "eth2", 1, 10);

    struct Data* data = getMonitorValues(now - 1);
    checkData(tc, data, now-1, 0, NULL, 3, 30);
    CuAssertTrue(tc, data->next == NULL);
}

void testMonitorDataOnAndAfterTs(CuTest *tc) {
    time_t now = makeTs("2009-01-01 10:00:00");
    setTime(now);
    emptyDb();
    addDbRow(now - 1, 1, "eth0", 1, 10);
    addDbRow(now - 1, 1, "eth1", 1, 10);
    addDbRow(now - 1, 1, "eth2", 1, 10);

    addDbRow(now - 2, 1, "eth0", 5, 50);
    addDbRow(now - 2, 1, "eth1", 5, 50);
    addDbRow(now - 2, 1, "eth2", 5, 50);

    struct Data* data = getMonitorValues(now - 2);
    checkData(tc, data, now-1, 0, NULL, 3, 30);

    data = data->next;
    checkData(tc, data, now-2, 0, NULL, 15, 150);

    CuAssertTrue(tc, data->next == NULL);
}

void testMonitorDataOnAndLongAfterTs(CuTest *tc) {
    time_t now  = makeTs("2009-01-01 10:00:00");
    time_t then = makeTs("2008-10-10 11:00:00");

    setTime(now);
    emptyDb();
    addDbRow(now, 1, "eth0", 1, 10);
    addDbRow(now, 1, "eth1", 1, 10);
    addDbRow(now, 1, "eth2", 1, 10);

    addDbRow(then + 3600, 3600, "eth0", 5, 50);
    addDbRow(then,        3600, "eth1", 6, 60);
    addDbRow(then - 3600, 3600, "eth2", 7, 70);

    struct Data* data = getMonitorValues(then);
    checkData(tc, data, now, 0, NULL, 3, 30);

    data = data->next;
    checkData(tc, data, then + 3600, 0, NULL, 5, 50);

    data = data->next;
    checkData(tc, data, then, 0, NULL, 6, 60);

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
