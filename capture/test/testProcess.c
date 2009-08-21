#include "test.h"
#include "common.h"
#include <string.h>
#include "capture.h"
#include "CuTest.h"

static doTestIsSameAddress(CuTest *tc, char* addr1, char* addr2, int expected);
static void checkBwData(CuTest *tc, struct BwData* data, int dl, int ul, char* addr);

void testIsSameAddress(CuTest *tc){
    doTestIsSameAddress(tc, "", "", 1);
    doTestIsSameAddress(tc, "a", "a", 1);
    doTestIsSameAddress(tc, "a", "", 0);
    doTestIsSameAddress(tc, "a", "b", 0);
    doTestIsSameAddress(tc, "eth0", "eth0", 1);
    doTestIsSameAddress(tc, "eth0", "lo0", 0);
    doTestIsSameAddress(tc, "12:34:56:78", "12:34:55:78", 0);
    doTestIsSameAddress(tc, "12:34:56:78", "12:34:56:78", 1);
}
static doTestIsSameAddress(CuTest *tc, char* addr1, char* addr2, int expected){
    struct BwData data1 = {0, 0, strlen(addr1), addr1, NULL};
    struct BwData data2 = {0, 0, strlen(addr2), addr2, NULL};
    int isSame = isSameAddress(&data1, &data2);
    CuAssertIntEquals(tc, expected, isSame);
}

void testExtractDiffsNull(CuTest *tc){
    struct BwData* diffs;

    diffs = extractDiffs(NULL, NULL);
    CuAssertPtrEquals(tc, NULL, diffs);

    struct BwData dataOk = {1, 2, 5, "eth0", NULL};
    diffs = extractDiffs(&dataOk, NULL);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(NULL, &dataOk);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsNoMatches(CuTest *tc){
    struct BwData data1 = {1, 2, 5, "eth0", NULL};
    struct BwData data2 = {2, 3, 5, "eth1", NULL};

    struct BwData* diffs;
    diffs = extractDiffs(&data1, &data2);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(&data2, &data1);
    CuAssertPtrEquals(tc, NULL, diffs);

    struct BwData data3 = {3, 4, 5, "eth2", &data1};
    struct BwData data4 = {4, 5, 5, "eth3", &data2};

    diffs = extractDiffs(&data3, &data4);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(&data4, &data3);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsNoChange(CuTest *tc){
    struct BwData data1c = {2, 3, 5, "eth2", NULL};
    struct BwData data1b = {2, 3, 5, "eth1", &data1c};
    struct BwData data1a = {1, 2, 5, "eth0", &data1b};

    struct BwData data2c = {1, 2, 5, "eth0", NULL};
    struct BwData data2b = {2, 3, 5, "eth1", &data2c};
    struct BwData data2a = {1, 2, 5, "eth3", &data2b};

    struct BwData* diffs;
    diffs = extractDiffs(&data1a, &data2a);
    CuAssertPtrEquals(tc, NULL, diffs);

    diffs = extractDiffs(&data2a, &data1a);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffs1Match(CuTest *tc){
    struct BwData data1c = {2, 3, 5, "eth2", NULL};
    struct BwData data1b = {2, 3, 5, "eth1", &data1c};
    struct BwData data1a = {1, 2, 5, "eth0", &data1b};

    struct BwData data2c = {2, 4, 5, "eth0", NULL};
    struct BwData data2b = {2, 3, 5, "eth3", &data2c};
    struct BwData data2a = {1, 2, 5, "eth4", &data2b};

    struct BwData* diffs;
    diffs = extractDiffs(&data1a, &data2a);
    checkBwData(tc, diffs, 1, 2, "eth0");
}

void testExtractDiffsValuesWrap(CuTest *tc){
    struct BwData data1 = {100, 200, 5, "eth0", NULL};
    struct BwData data2 = {1,   2,   5, "eth0", NULL};

    struct BwData* diffs = extractDiffs(&data1, &data2);
    CuAssertPtrEquals(tc, NULL, diffs);
}

void testExtractDiffsMultiMatch(CuTest *tc){
    struct BwData data1d = {2, 4, 5, "eth4", NULL};
    struct BwData data1c = {2, 3, 5, "eth2", &data1d};
    struct BwData data1b = {1, 2, 5, "eth1", &data1c};
    struct BwData data1a = {0, 0, 5, "eth0", &data1b};

    struct BwData data2d = {2, 3, 5, "eth3", NULL};
    struct BwData data2c = {2, 4, 5, "eth2", &data2d};
    struct BwData data2b = {2, 5, 5, "eth1", &data2c};
    struct BwData data2a = {5, 7, 5, "eth0", &data2b};


    struct BwData* diffs;
    diffs = extractDiffs(&data1a, &data2a);
    checkBwData(tc, diffs, 5, 7, "eth0");

    diffs = diffs->next;
    checkBwData(tc, diffs, 1, 3, "eth1");

    diffs = diffs->next;
    checkBwData(tc, diffs, 0, 1, "eth2");

    CuAssertPtrEquals(tc, NULL, diffs->next);
}

static void checkBwData(CuTest *tc, struct BwData* data, int dl, int ul, char* addr){
    CuAssertIntEquals(tc, dl, data->dl);
    CuAssertIntEquals(tc, ul, data->ul);
    CuAssertStrEquals(tc, addr, data->addr);
}

CuSuite* processGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testIsSameAddress);
    SUITE_ADD_TEST(suite, testExtractDiffsNull);
    SUITE_ADD_TEST(suite, testExtractDiffsNoMatches);
    SUITE_ADD_TEST(suite, testExtractDiffsNoChange);
    SUITE_ADD_TEST(suite, testExtractDiffs1Match);
    SUITE_ADD_TEST(suite, testExtractDiffsValuesWrap);
    SUITE_ADD_TEST(suite, testExtractDiffsMultiMatch);
    return suite;
}
