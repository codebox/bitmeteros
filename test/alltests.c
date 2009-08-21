#include "CuTest.h"

CuSuite* sqlGetSuite();

void RunAllTests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite   = CuSuiteNew();

    CuSuiteAddSuite(suite, sqlGetSuite());
    CuSuiteAddSuite(suite, processGetSuite());
    CuSuiteAddSuite(suite, commonGetSuite());
    CuSuiteAddSuite(suite, timeGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(void) {
    setup();
    RunAllTests();
}
