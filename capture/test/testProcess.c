#include "test.h"
#include "common.h"
#include <string.h>
#include "capture.h"
#include "CuTest.h"

static doTestIsSameAddress(CuTest *tc, char* addr1, char* addr2, int expected);
static void checkData(CuTest *tc, struct Data* data, int dl, int ul, char* addr);


void testExtractDiffsNull(CuTest *tc){
    struct Data* diffs;

    diffs = extractDiffs(NULL, NULL);
    CuAssertPtrEquals(tc, NULL, diffs);

    struct Data dataOk = {0, 1, 2, 5, "eth0", NULL};
    diffs = extractDiffs(&dataOk, NULL);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(NULL, &dataOk);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsNoMatches(CuTest *tc){
    struct Data data1 = {0, 1, 2, 5, "eth0", NULL};
    struct Data data2 = {0, 2, 3, 5, "eth1", NULL};

    struct Data* diffs;
    diffs = extractDiffs(&data1, &data2);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(&data2, &data1);
    CuAssertPtrEquals(tc, NULL, diffs);

    struct Data data3 = {0, 3, 4, 5, "eth2", &data1};
    struct Data data4 = {0, 4, 5, 5, "eth3", &data2};

    diffs = extractDiffs(&data3, &data4);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(&data4, &data3);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsNoChange(CuTest *tc){
    struct Data data1c = {0, 2, 3, 5, "eth2", NULL};
    struct Data data1b = {0, 2, 3, 5, "eth1", &data1c};
    struct Data data1a = {0, 1, 2, 5, "eth0", &data1b};

    struct Data data2c = {0, 1, 2, 5, "eth0", NULL};
    struct Data data2b = {0, 2, 3, 5, "eth1", &data2c};
    struct Data data2a = {0, 1, 2, 5, "eth3", &data2b};

    struct Data* diffs;
    diffs = extractDiffs(&data1a, &data2a);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(&data2a, &data1a);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffs1Match(CuTest *tc){
    struct Data data1c = {0, 2, 3, 5, "eth2", NULL};
    struct Data data1b = {0, 2, 3, 5, "eth1", &data1c};
    struct Data data1a = {0, 1, 2, 5, "eth0", &data1b};

    struct Data data2c = {0, 2, 4, 5, "eth0", NULL};
    struct Data data2b = {0, 2, 3, 5, "eth3", &data2c};
    struct Data data2a = {0, 1, 2, 5, "eth4", &data2b};

    struct Data* diffs;
    diffs = extractDiffs(&data1a, &data2a);
    checkData(tc, diffs, 1, 2, "eth0");
}

void testExtractDiffsValuesWrap(CuTest *tc){
    struct Data data1 = {0, 100, 200, 5, "eth0", NULL};
    struct Data data2 = {0, 1,   2,   5, "eth0", NULL};

    struct Data* diffs = extractDiffs(&data1, &data2);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsMultiMatch(CuTest *tc){
    struct Data data1d = {0, 2, 4, 5, "eth4", NULL};
    struct Data data1c = {0, 2, 3, 5, "eth2", &data1d};
    struct Data data1b = {0, 1, 2, 5, "eth1", &data1c};
    struct Data data1a = {0, 0, 0, 5, "eth0", &data1b};

    struct Data data2d = {0, 2, 3, 5, "eth3", NULL};
    struct Data data2c = {0, 2, 4, 5, "eth2", &data2d};
    struct Data data2b = {0, 2, 5, 5, "eth1", &data2c};
    struct Data data2a = {0, 5, 7, 5, "eth0", &data2b};


    struct Data* diffs;
    diffs = extractDiffs(&data1a, &data2a);
    checkData(tc, diffs, 5, 7, "eth0");

    diffs = diffs->next;
    checkData(tc, diffs, 1, 3, "eth1");

    diffs = diffs->next;
    checkData(tc, diffs, 0, 1, "eth2");

    CuAssertPtrEquals(tc, NULL, diffs->next);
}

static void checkData(CuTest *tc, struct Data* data, int dl, int ul, char* ad){
    CuAssertIntEquals(tc, dl, data->dl);
    CuAssertIntEquals(tc, ul, data->ul);
    CuAssertStrEquals(tc, ad, data->ad);
}

CuSuite* processGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testExtractDiffsNull);
    SUITE_ADD_TEST(suite, testExtractDiffsNoMatches);
    SUITE_ADD_TEST(suite, testExtractDiffsNoChange);
    SUITE_ADD_TEST(suite, testExtractDiffs1Match);
    SUITE_ADD_TEST(suite, testExtractDiffsValuesWrap);
    SUITE_ADD_TEST(suite, testExtractDiffsMultiMatch);
    return suite;
}
