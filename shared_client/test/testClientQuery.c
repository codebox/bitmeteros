#include "test.h"
#include "common.h"
#include "client.h"
#include "CuTest.h"

static void checkQueryResults(CuTest *tc, struct Data* data, int dl, int ul);

void testQueryEmptyDb(CuTest *tc) {
    emptyDb();

    struct Data* data = getQueryValues(makeTs("2009-03-03 08:00:00"), makeTs("2009-03-03 11:00:00"), QUERY_GROUP_HOURS);
    CuAssertTrue(tc, data == NULL);
}

void testQueryNoDataInRange(CuTest *tc) {
    emptyDb();

    addDbRow(makeTs("2009-03-03 08:00:00"), 3600, "eth0", 1, 2); // covers 07:00-08:00 so out of range
    addDbRow(makeTs("2009-03-03 12:00:00"), 3600, "eth0", 1, 2); // covers 11:00-12:00 so out of range

    struct Data* data = getQueryValues(makeTs("2009-03-03 08:00:00"), makeTs("2009-03-03 11:00:00"), QUERY_GROUP_HOURS);
    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeHours(CuTest *tc) {
    emptyDb();

    addDbRow(makeTs("2009-03-03 08:00:00"), 3600, "eth0", 1, 11); // covers 07:00-08:00 so out of range
    addDbRow(makeTs("2009-03-03 09:00:00"), 3600, "eth0", 2, 12); // covers 08:00-09:00 so in range
    addDbRow(makeTs("2009-03-03 11:00:00"), 3600, "eth0", 3, 13); // covers 10:00-11:00 so in range
    addDbRow(makeTs("2009-03-03 12:00:00"), 3600, "eth0", 4, 14); // covers 11:00-12:00 so out of range

    struct Data* data = getQueryValues(makeTs("2009-03-03 08:00:00"), makeTs("2009-03-03 11:00:00"), QUERY_GROUP_HOURS);

    checkData(tc, data, makeTs("2009-03-03 09:00:00"), 3600, NULL, 2, 12);
    data = data->next;

    checkData(tc, data, makeTs("2009-03-03 11:00:00"), 3600, NULL, 3, 13);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeDays(CuTest *tc) {
    emptyDb();

    addDbRow(makeTs("2009-03-03 00:00:00"), 3600, "eth0", 1, 11); // out of range

    addDbRow(makeTs("2009-03-03 01:00:00"), 3600, "eth0", 2, 12);
    addDbRow(makeTs("2009-03-03 23:00:00"), 3600, "eth0", 3, 13); // data for the 3rd
    addDbRow(makeTs("2009-03-04 00:00:00"), 3600, "eth0", 4, 14);

    addDbRow(makeTs("2009-03-04 01:00:00"), 3600, "eth0", 5, 15);
    addDbRow(makeTs("2009-03-04 23:00:00"), 3600, "eth0", 6, 16); // data for the 4th
    addDbRow(makeTs("2009-03-05 00:00:00"), 3600, "eth0", 7, 17);

    addDbRow(makeTs("2009-03-05 01:00:00"), 3600, "eth0", 8, 18); // out of range

    struct Data* data = getQueryValues(makeTs("2009-03-03 00:00:00"), makeTs("2009-03-05 00:00:00"), QUERY_GROUP_DAYS);

    checkData(tc, data, makeTs("2009-03-04 00:00:00"), 3600 * 24, NULL, 9, 39); // data for the 3rd, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTs("2009-03-05 00:00:00"), 3600 * 24, NULL, 18, 48); // data for the 4th
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeMonths(CuTest *tc) {
    emptyDb();

    addDbRow(makeTs("2009-03-01 00:00:00"), 3600, "eth0", 1, 101); // out of range

    addDbRow(makeTs("2009-03-01 01:00:00"), 3600, "eth0", 2, 102);
    addDbRow(makeTs("2009-03-30 23:00:00"), 3600, "eth0", 3, 103); // data for March
    addDbRow(makeTs("2009-04-01 00:00:00"), 3600, "eth0", 4, 104);

    addDbRow(makeTs("2009-04-01 01:00:00"), 3600, "eth0", 5, 105);
    addDbRow(makeTs("2009-04-30 23:00:00"), 3600, "eth0", 6, 106); // data for April
    addDbRow(makeTs("2009-05-01 00:00:00"), 3600, "eth0", 7, 107);

    addDbRow(makeTs("2009-05-01 01:00:00"), 3600, "eth0", 8, 108); // out of range

    struct Data* data = getQueryValues(makeTs("2009-03-01 00:00:00"), makeTs("2009-05-01 00:00:00"), QUERY_GROUP_MONTHS);

    checkData(tc, data, makeTs("2009-04-01 00:00:00"), 31 * 3600 * 24, NULL, 9, 309); // data for March, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTs("2009-05-01 00:00:00"), 30 * 3600 * 24, NULL, 18, 318); // data for April
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryDataInRangeYears(CuTest *tc) {
    emptyDb();

    addDbRow(makeTs("2007-01-01 00:00:00"), 3600, "eth0", 1, 101); // out of range

    addDbRow(makeTs("2007-01-01 01:00:00"), 3600, "eth0", 2, 102);
    addDbRow(makeTs("2007-12-31 23:00:00"), 3600, "eth0", 3, 103); // data for 2007
    addDbRow(makeTs("2008-01-01 00:00:00"), 3600, "eth0", 4, 104);

    addDbRow(makeTs("2008-01-01 01:00:00"), 3600, "eth0", 5, 105);
    addDbRow(makeTs("2008-12-31 23:00:00"), 3600, "eth0", 6, 106); // data for 2008
    addDbRow(makeTs("2009-01-01 00:00:00"), 3600, "eth0", 7, 107);

    addDbRow(makeTs("2009-01-01 01:00:00"), 3600, "eth0", 8, 108); // out of range

    struct Data* data = getQueryValues(makeTs("2007-01-01 00:00:00"), makeTs("2009-01-01 00:00:00"), QUERY_GROUP_YEARS);

    checkData(tc, data, makeTs("2008-01-01 00:00:00"), 365 * 3600 * 24, NULL, 9, 309); // data for 2007, remember ts is the END of the interval
    data = data->next;

    checkData(tc, data, makeTs("2009-01-01 00:00:00"), 366 * 3600 * 24, NULL, 18, 318); // data for 2008
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

void testQueryLargeQueryRange(CuTest *tc) {
    emptyDb();

    addDbRow(makeTs("2009-03-03 08:00:00"), 3600, "eth0", 1, 11);
    addDbRow(makeTs("2009-03-03 09:00:00"), 3600, "eth0", 2, 12);
    addDbRow(makeTs("2009-03-03 11:00:00"), 3600, "eth0", 3, 13);
    addDbRow(makeTs("2009-03-03 12:00:00"), 3600, "eth0", 4, 14);

    struct Data* data = getQueryValues(makeTs("2000-01-01 00:00:00"), makeTs("2019-01-01 00:00:00"), QUERY_GROUP_TOTAL);

    checkData(tc, data, makeTs("2009-03-03 12:00:00"), 14401, NULL, 10, 50);
    data = data->next;

    CuAssertTrue(tc, data == NULL);
}

CuSuite* clientQueryGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testQueryEmptyDb);
    SUITE_ADD_TEST(suite, testQueryNoDataInRange);
    SUITE_ADD_TEST(suite, testQueryDataInRangeHours);
    SUITE_ADD_TEST(suite, testQueryDataInRangeDays);
    SUITE_ADD_TEST(suite, testQueryDataInRangeMonths);
    SUITE_ADD_TEST(suite, testQueryDataInRangeYears);
    SUITE_ADD_TEST(suite, testQueryLargeQueryRange);
    return suite;
}
