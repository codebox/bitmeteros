#include "common.h"
#include "test.h"
#include "client.h"
#include "CuTest.h"

static void onDumpRow(struct Data* data);
struct Data* dumpResult;

void testDumpEmptyDb(CuTest *tc) {
    emptyDb();
    dumpResult = NULL;
    getDumpValues(&onDumpRow);
    CuAssertTrue(tc, dumpResult == NULL);
}

void testDumpOneEntry(CuTest *tc) {
    emptyDb();
    dumpResult = NULL;

    addDbRow(1234, 1, "eth0", 1, 2);
    getDumpValues(&onDumpRow);
    checkData(tc, dumpResult, 1234, 1, "eth0", 1, 2);

    dumpResult = dumpResult->next;
    CuAssertTrue(tc, dumpResult == NULL);
}


static void onDumpRow(struct Data* data) {
    appendData(&dumpResult, data);
}

CuSuite* clientDumpGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testDumpEmptyDb);
    SUITE_ADD_TEST(suite, testDumpOneEntry);
    return suite;
}
