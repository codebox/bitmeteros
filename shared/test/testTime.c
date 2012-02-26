#include <stdlib.h>
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <string.h>
#include <time.h>
#include <test.h> 
#include <cmockery.h> 
#include "common.h"

/*
Contains unit tests for the 'time' module.
*/

void makeTm(struct tm *t, int year, int month, int day, int hour, int min, int sec){
    t->tm_sec  = sec;
    t->tm_min  = min;
    t->tm_hour = hour;
    t->tm_mday = day;
    t->tm_mon  = month - 1;
    t->tm_year = year - 1900;
    t->tm_isdst = -1;
}
void testTimeGm(void** state){
    struct tm t;
    
    makeTm(&t, 2010, 1, 1, 0, 0, 0);
    assert_int_equal(1262304000, timegm(&t));

    makeTm(&t, 2010, 7, 1, 0, 0, 0);
    assert_int_equal(1277942400, timegm(&t));
}

void testGetCurrentYearForTs(void** state){
 // Check that the 'getCurrentYearForTs' function correctly calculates the start of the current year for various timestamps
    assert_int_equal(makeTsUtc("2009-01-01 00:00:00"), getCurrentLocalYearForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2009-01-01 00:00:00"), getCurrentLocalYearForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2009-01-01 00:00:00"), getCurrentLocalYearForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetCurrentMonthForTs(void** state){
 // Check that the 'getCurrentMonthForTs' function correctly calculates the start of the current month for various timestamps
    assert_int_equal(makeTsUtc("2009-01-01 00:00:00"), getCurrentLocalMonthForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2009-03-01 00:00:00"), getCurrentLocalMonthForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2009-12-01 00:00:00"), getCurrentLocalMonthForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetCurrentDayForTs(void** state){
 // Check that the 'getCurrentDayForTs' function correctly calculates the start of the current day for various timestamps
    assert_int_equal(makeTsUtc("2009-01-01 00:00:00"), getCurrentLocalDayForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2009-03-24 00:00:00"), getCurrentLocalDayForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2009-12-31 00:00:00"), getCurrentLocalDayForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextYearForTs(void** state){
 // Check that the 'getNextYearForTs' function correctly calculates the start of the next year for various timestamps
    assert_int_equal(makeTsUtc("1971-01-01 00:00:00"), getNextYearForTs(makeTsUtc("1970-05-26 10:01:00")));
    assert_int_equal(makeTsUtc("2010-01-01 00:00:00"), getNextYearForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2010-01-01 00:00:00"), getNextYearForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2010-01-01 00:00:00"), getNextYearForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextMonthForTs(void** state){
 // Check that the 'getNextMonthForTs' function correctly calculates the start of the next month for various timestamps
    assert_int_equal(makeTsUtc("1970-06-01 00:00:00"), getNextMonthForTs(makeTsUtc("1970-05-26 10:01:00")));
    assert_int_equal(makeTsUtc("2009-02-01 00:00:00"), getNextMonthForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2009-04-01 00:00:00"), getNextMonthForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2010-01-01 00:00:00"), getNextMonthForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextDayForTs(void** state){
 // Check that the 'getNextDayForTs' function correctly calculates the start of the next day for various timestamps
    assert_int_equal(makeTsUtc("1970-05-27 00:00:00"), getNextDayForTs(makeTsUtc("1970-05-26 10:01:00")));
    assert_int_equal(makeTsUtc("2009-01-02 00:00:00"), getNextDayForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2009-03-25 00:00:00"), getNextDayForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2010-01-01 00:00:00"), getNextDayForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextHourForTs(void** state){
 // Check that the 'getNextHourForTs' function correctly calculates the start of the next hour for various timestamps
    assert_int_equal(makeTsUtc("1970-05-26 11:00:00"), getNextHourForTs(makeTsUtc("1970-05-26 10:01:00")));
    assert_int_equal(makeTsUtc("2009-01-01 01:00:00"), getNextHourForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2009-03-24 20:00:00"), getNextHourForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2010-01-01 00:00:00"), getNextHourForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testGetNextMinForTs(void** state){
 // Check that the 'getNextMinForTs' function correctly calculates the start of the next minute for various timestamps
    assert_int_equal(makeTsUtc("1970-05-26 10:02:00"), getNextMinForTs(makeTsUtc("1970-05-26 10:01:00")));
    assert_int_equal(makeTsUtc("2009-01-01 00:01:00"), getNextMinForTs(makeTsUtc("2009-01-01 00:00:00")));
    assert_int_equal(makeTsUtc("2009-03-24 19:13:00"), getNextMinForTs(makeTsUtc("2009-03-24 19:12:01")));
    assert_int_equal(makeTsUtc("2010-01-01 00:00:00"), getNextMinForTs(makeTsUtc("2009-12-31 23:59:59")));
}

void testAddToDate(void** state){
 // Check that the 'addToDate' function correctly adds various different values to a timestamp
    assert_int_equal(makeTsUtc("1980-05-26 11:01:00"), addToDate(makeTsUtc("1980-05-26 10:01:00"), 'h', 1));
    assert_int_equal(makeTsUtc("1980-05-28 10:01:00"), addToDate(makeTsUtc("1980-05-26 10:01:00"), 'd', 2));
    assert_int_equal(makeTsUtc("1980-08-26 10:01:00"), addToDate(makeTsUtc("1980-05-26 10:01:00"), 'm', 3));
    assert_int_equal(makeTsUtc("1984-05-26 10:01:00"), addToDate(makeTsUtc("1980-05-26 10:01:00"), 'y', 4));
}

void testNormaliseTm(void** state){
    struct tm t;
    
 // Month overflows
    makeTm(&t, 2011, 13, 30, 10, 0, 0);
    assert_int_equal(13   - 1, t.tm_mon);
    assert_int_equal(2011 - 1900, t.tm_year);
    normaliseTm(&t);
    assert_int_equal(1    - 1, t.tm_mon);
    assert_int_equal(2012 - 1900, t.tm_year);
    
 // Day overflows
    makeTm(&t, 2011, 10, 32, 10, 0, 0);
    assert_int_equal(32,     t.tm_mday);
    assert_int_equal(10 - 1, t.tm_mon);
    normaliseTm(&t);
    assert_int_equal(1,     t.tm_mday);
    assert_int_equal(11 - 1, t.tm_mon);
    
 // Hour overflows
    makeTm(&t, 2011, 10, 12, 24, 0, 0);
    assert_int_equal(24, t.tm_hour);
    assert_int_equal(12, t.tm_mday);
    normaliseTm(&t);
    assert_int_equal(0,  t.tm_hour);
    assert_int_equal(13, t.tm_mday);
    
 // Minute overflows
    makeTm(&t, 2011, 10, 12, 22, 60, 0);
    assert_int_equal(22, t.tm_hour);
    assert_int_equal(60, t.tm_min);
    normaliseTm(&t);
    assert_int_equal(23, t.tm_hour);
    assert_int_equal(0,  t.tm_min);

 // Second overflows
    makeTm(&t, 2011, 10, 12, 22, 30, 60);
    assert_int_equal(60, t.tm_sec);
    assert_int_equal(30, t.tm_min);
    normaliseTm(&t);
    assert_int_equal(0,  t.tm_sec);
    assert_int_equal(31, t.tm_min);
}
