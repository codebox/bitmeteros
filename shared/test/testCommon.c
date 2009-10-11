#include "test.h"
#include "common.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "capture.h"
#include "CuTest.h"

static void doTestFormatAmount(CuTest *, BW_INT , char* , char* , char *, char* );

void testFormatAmounts(CuTest *tc){
    doTestFormatAmount(tc, 0, "0.00 B ", "0.00 bytes", "0.00 B ", "0.00 bytes");
    doTestFormatAmount(tc, 1, "1.00 B ", "1.00 bytes", "1.00 B ", "1.00 bytes");

    doTestFormatAmount(tc, 999,   "999.00 B ",   "999.00 bytes", "999.00 B ",   "999.00 bytes");
    doTestFormatAmount(tc, 1000, "1000.00 B ",  "1000.00 bytes",   "1.00 kB", "1.00 kilobytes");
    doTestFormatAmount(tc, 1023, "1023.00 B ",  "1023.00 bytes",   "1.02 kB", "1.02 kilobytes");
    doTestFormatAmount(tc, 1024,    "1.00 kB", "1.00 kilobytes",   "1.02 kB", "1.02 kilobytes");

    doTestFormatAmount(tc,  999999,   "976.56 kB",  "976.56 kilobytes", "1000.00 kB",   "1000.00 kilobytes");
    doTestFormatAmount(tc, 1000000 ,  "976.56 kB",  "976.56 kilobytes",    "1.00 MB",      "1.00 megabytes");
    doTestFormatAmount(tc, 1048575,  "1024.00 kB", "1024.00 kilobytes",    "1.05 MB",      "1.05 megabytes");
    doTestFormatAmount(tc, 1048576,     "1.00 MB",    "1.00 megabytes",    "1.05 MB",      "1.05 megabytes");

    doTestFormatAmount(tc,  999999999,   "953.67 MB",  "953.67 megabytes", "1000.00 MB",   "1000.00 megabytes");
    doTestFormatAmount(tc, 1000000000 ,  "953.67 MB",  "953.67 megabytes",    "1.00 GB",      "1.00 gigabytes");
    doTestFormatAmount(tc, 1073741823,  "1024.00 MB", "1024.00 megabytes",    "1.07 GB",      "1.07 gigabytes");
    doTestFormatAmount(tc, 1073741824,     "1.00 GB",    "1.00 gigabytes",    "1.07 GB",      "1.07 gigabytes");

    doTestFormatAmount(tc,  999999999999,  "931.32 GB",  "931.32 gigabytes", "1000.00 GB",   "1000.00 gigabytes");
    doTestFormatAmount(tc, 1000000000000,  "931.32 GB",  "931.32 gigabytes",    "1.00 TB",      "1.00 terabytes");
    doTestFormatAmount(tc, 1099511627775, "1024.00 GB", "1024.00 gigabytes",    "1.10 TB",      "1.10 terabytes");
    doTestFormatAmount(tc, 1099511627776,    "1.00 TB",    "1.00 terabytes",    "1.10 TB",      "1.10 terabytes");

    doTestFormatAmount(tc,  999999999999999,  "909.49 TB",  "909.49 terabytes", "1000.00 TB",   "1000.00 terabytes");
    doTestFormatAmount(tc, 1000000000000000,  "909.49 TB",  "909.49 terabytes",    "1.00 PB",      "1.00 petabytes");
    doTestFormatAmount(tc, 1125899906842623, "1024.00 TB", "1024.00 terabytes",    "1.13 PB",      "1.13 petabytes");
    doTestFormatAmount(tc, 1125899906842624,    "1.00 PB",    "1.00 petabytes",    "1.13 PB",      "1.13 petabytes");

    doTestFormatAmount(tc,  999999999999999999,  "888.18 PB",  "888.18 petabytes", "1000.00 PB",   "1000.00 petabytes");
    doTestFormatAmount(tc, 1000000000000000000,  "888.18 PB",  "888.18 petabytes",    "1.00 EB",      "1.00 exabytes");
    doTestFormatAmount(tc, 1152921504606846975, "1024.00 PB", "1024.00 petabytes",    "1.15 EB",      "1.15 exabytes");
    doTestFormatAmount(tc, 1152921504606846976,    "1.00 EB",     "1.00 exabytes",    "1.15 EB",      "1.15 exabytes");
}

static void doTestFormatAmount(CuTest *tc, BW_INT amount, char* binaryShort, char* binaryLong, char *decimalShort, char* decimalLong){
    char txt[24];

    formatAmount(amount, 1, 1, txt);
    CuAssertStrEquals(tc, binaryShort, txt);

    formatAmount(amount, 1, 0, txt);
    CuAssertStrEquals(tc, binaryLong, txt);

    formatAmount(amount, 0, 1, txt);
    CuAssertStrEquals(tc, decimalShort, txt);

    formatAmount(amount, 0, 0, txt);
    CuAssertStrEquals(tc, decimalLong, txt);

}

void testToTime(CuTest *tc){
    doTestToTime(tc, 0,  "00:00:00");
    doTestToTime(tc, 1,  "00:00:01");
    doTestToTime(tc, 60, "00:01:00");

    doTestToTime(tc, makeTs("2000-01-01 00:00:00"), "00:00:00");
    doTestToTime(tc, makeTs("2000-01-01 00:00:01"), "00:00:01");
    doTestToTime(tc, makeTs("2000-01-01 00:02:01"), "00:02:01");
    doTestToTime(tc, makeTs("2000-01-01 00:59:59"), "00:59:59");
    doTestToTime(tc, makeTs("2000-01-01 12:12:12"), "12:12:12");
    doTestToTime(tc, makeTs("2000-01-01 23:59:59"), "23:59:59");
}
void doTestToTime(CuTest *tc, int ts, char* expected){
    char actual[20];
    toTime(actual, ts);
    CuAssertStrEquals(tc, expected, actual);
}
void testToDate(CuTest *tc){
    doTestToDate(tc, 0,  "1970-01-01");

    doTestToDate(tc, makeTs("2000-01-01 00:00:00"), "2000-01-01");
    doTestToDate(tc, makeTs("2008-12-31 00:00:00"), "2008-12-31");
    doTestToDate(tc, makeTs("2008-02-29 00:00:00"), "2008-02-29");
}
void doTestToDate(CuTest *tc, int ts, char* expected){
    char actual[20];
    toDate(actual, ts);
    CuAssertStrEquals(tc, expected, actual);
}

CuSuite* commonGetSuite() {
    putenv("TZ=GMT"); // Need this because the date/time fns use localtime
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testFormatAmounts);
    SUITE_ADD_TEST(suite, testToTime);
    SUITE_ADD_TEST(suite, testToDate);
    return suite;
}
