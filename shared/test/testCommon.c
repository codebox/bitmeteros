#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "time.h"

/*
Contains unit tests for the 'common' module.
*/

static void doTestFormatAmount(BW_INT , char* , char* , char *, char* );
static void doTestToTime(int ts, char* expected);
static void doTestToDate(int ts, char* expected);

void testFormatAmounts(void **state){
 // Check that various amounts are formatted correctly, using all combinations of binary/decimal and full/abbreviated units
    doTestFormatAmount(0, "0.00 B ", "0.00 bytes", "0.00 B ", "0.00 bytes");
    doTestFormatAmount(1, "1.00 B ", "1.00 bytes", "1.00 B ", "1.00 bytes");

    doTestFormatAmount(999,   "999.00 B ",   "999.00 bytes", "999.00 B ",   "999.00 bytes");
    doTestFormatAmount(1000, "1000.00 B ",  "1000.00 bytes",   "1.00 kB", "1.00 kilobytes");
    doTestFormatAmount(1023, "1023.00 B ",  "1023.00 bytes",   "1.02 kB", "1.02 kilobytes");
    doTestFormatAmount(1024,    "1.00 kB", "1.00 kilobytes",   "1.02 kB", "1.02 kilobytes");

    doTestFormatAmount( 999999,   "976.56 kB",  "976.56 kilobytes", "1000.00 kB",   "1000.00 kilobytes");
    doTestFormatAmount(1000000 ,  "976.56 kB",  "976.56 kilobytes",    "1.00 MB",      "1.00 megabytes");
    doTestFormatAmount(1048575,  "1024.00 kB", "1024.00 kilobytes",    "1.05 MB",      "1.05 megabytes");
    doTestFormatAmount(1048576,     "1.00 MB",    "1.00 megabytes",    "1.05 MB",      "1.05 megabytes");

    doTestFormatAmount( 999999999,   "953.67 MB",  "953.67 megabytes", "1000.00 MB",   "1000.00 megabytes");
    doTestFormatAmount(1000000000 ,  "953.67 MB",  "953.67 megabytes",    "1.00 GB",      "1.00 gigabytes");
    doTestFormatAmount(1073741823,  "1024.00 MB", "1024.00 megabytes",    "1.07 GB",      "1.07 gigabytes");
    doTestFormatAmount(1073741824,     "1.00 GB",    "1.00 gigabytes",    "1.07 GB",      "1.07 gigabytes");

    doTestFormatAmount( 999999999999llu,  "931.32 GB",  "931.32 gigabytes", "1000.00 GB",   "1000.00 gigabytes");
    doTestFormatAmount(1000000000000llu,  "931.32 GB",  "931.32 gigabytes",    "1.00 TB",      "1.00 terabytes");
    doTestFormatAmount(1099511627775llu, "1024.00 GB", "1024.00 gigabytes",    "1.10 TB",      "1.10 terabytes");
    doTestFormatAmount(1099511627776llu,    "1.00 TB",    "1.00 terabytes",    "1.10 TB",      "1.10 terabytes");

    doTestFormatAmount( 999999999999999llu,  "909.49 TB",  "909.49 terabytes", "1000.00 TB",   "1000.00 terabytes");
    doTestFormatAmount(1000000000000000llu,  "909.49 TB",  "909.49 terabytes",    "1.00 PB",      "1.00 petabytes");
    doTestFormatAmount(1125899906842623llu, "1024.00 TB", "1024.00 terabytes",    "1.13 PB",      "1.13 petabytes");
    doTestFormatAmount(1125899906842624llu,    "1.00 PB",    "1.00 petabytes",    "1.13 PB",      "1.13 petabytes");

    doTestFormatAmount( 999999999999999999llu,  "888.18 PB",  "888.18 petabytes", "1000.00 PB",   "1000.00 petabytes");
    doTestFormatAmount(1000000000000000000llu,  "888.18 PB",  "888.18 petabytes",    "1.00 EB",      "1.00 exabytes");
    doTestFormatAmount(1152921504606846975llu, "1024.00 PB", "1024.00 petabytes",    "1.15 EB",      "1.15 exabytes");
    doTestFormatAmount(1152921504606846976llu,    "1.00 EB",     "1.00 exabytes",    "1.15 EB",      "1.15 exabytes");
}

static void doTestFormatAmount(BW_INT amount, char* binaryShort, char* binaryLong, char *decimalShort, char* decimalLong){
 // Helper function for checking value formatting using all combinations of binary/decimal and full/abbreviated units
    char txt[24];

    formatAmount(amount, 1, 1, txt);
    assert_string_equal(binaryShort, txt);

    formatAmount(amount, 1, 0, txt);
    assert_string_equal(binaryLong, txt);

    formatAmount(amount, 0, 1, txt);
    assert_string_equal(decimalShort, txt);

    formatAmount(amount, 0, 0, txt);
    assert_string_equal(decimalLong, txt);
}

void testToTime(void **state){
 // Check that the 'toTime' function correctly extracts the time component from various timestamps
    doTestToTime(makeTs("2000-01-01 00:00:00"), "00:00:00");
    doTestToTime(makeTs("2000-01-01 00:00:01"), "00:00:01");
    doTestToTime(makeTs("2000-01-01 00:02:01"), "00:02:01");
    doTestToTime(makeTs("2000-07-01 00:59:59"), "00:59:59");
    doTestToTime(makeTs("2000-07-01 12:12:12"), "12:12:12");
    doTestToTime(makeTs("2000-07-01 23:59:59"), "23:59:59");
}
static void doTestToTime(int ts, char* expected){
 // Helper function to check that the 'toTime' function correctly extracts the time component of a timestamp
    char actual[20];
    toTime(actual, ts);
    assert_string_equal(expected, actual);
}

void testToDate(void **state){
 // Check that the 'toDate' function correctly extracts the date component from various timestamps
    doTestToDate(0, "1970-01-01");

    doTestToDate(makeTs("2000-01-01 00:00:00"), "2000-01-01");
    doTestToDate(makeTs("2008-12-31 00:00:00"), "2008-12-31");
    doTestToDate(makeTs("2008-02-29 00:00:00"), "2008-02-29");
    doTestToDate(makeTs("2010-07-29 00:00:00"), "2010-07-29");
}

void testMakeHexString(void **state){
    char bytes1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    char hex1[36];

    makeHexString(hex1, bytes1, 17);
    assert_string_equal("000102030405060708090A0B0C0D0E0F10", hex1);

    char bytes2[] = {};
    char hex2[2];

    makeHexString(hex2, bytes2, 0);
    assert_string_equal("", hex2);

    char bytes3[] = {0, 12, 241, 86, 152, 173};
    char hex3[13];

    makeHexString(hex3, bytes3, 6);
    assert_string_equal("000CF15698AD", hex3);
}

static void doTestToDate(int ts, char* expected){
 // Helper function to check that the 'toDate' function correctly extracts the date component of a timestamp
    char actual[20];
    toDate(actual, ts);
    assert_string_equal(expected, actual);
}

void testStrToLong(void **state){
    assert_true(strToLong("", -1) == -1);
    assert_true(strToLong("asd", -1) == -1);
    assert_true(strToLong("12a", -1) == -1);
    assert_true(strToLong(" 123", -1) == 123);
    assert_true(strToLong(NULL, -1) == -1);
}

static void checkReplace(char* expected, char* src, char* target, char* replacement){
    char* result = replace(src, target, replacement);
    if (result != NULL){
        assert_string_equal(expected, result);
        free(result);   
    } else {
        assert_true(expected == NULL);  
    }
}
void testReplace(void** state){
    checkReplace(NULL,  NULL,  "a",  "b");
    
    checkReplace("abc", "abc", NULL, "x");
    
    checkReplace("bc", "abc", "a",  NULL);
    checkReplace("ac", "abc", "b",  NULL);
    checkReplace("ab", "abc", "c",  NULL);
    
    checkReplace("x",   "abc", "abc",  "x");
    checkReplace("abc", "abc", "X",  "b");
    checkReplace("aabbccdd", "abc", "abc",  "aabbccdd");
}

void testStrAppend(void** state){
    char* result;
    
    result = strAppend("text", NULL);
    assert_string_equal("text", result);
    free(result);
    
    result = strAppend("1", "2", NULL);
    assert_string_equal("12", result);
    free(result);
    
    result = strAppend("123", "", "4", "56789", NULL);
    assert_string_equal("123456789", result);
    free(result);

    char* txt = strdup("sat");
    result = strAppend("", "the cat ", "", txt, " on the mat", NULL);
    free(txt);
    assert_string_equal("the cat sat on the mat", result);
    free(result);
}