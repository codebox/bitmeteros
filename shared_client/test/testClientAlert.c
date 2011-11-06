#ifdef _WIN32
    #define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include <string.h>
#include "common.h"
#include "client.h"
#include "test.h"

static time_t makeTsFromParts(int y, int m, int d, int h){
 /*struct tm {
        int tm_sec;     Seconds: 0-59 (K&R says 0-61?) 
        int tm_min;     Minutes: 0-59 
        int tm_hour;    Hours since midnight: 0-23 
        int tm_mday;    Day of the month: 1-31 
        int tm_mon;     Months *since* january: 0-11 
        int tm_year;    Years since 1900 
        int tm_wday;    Days since Sunday (0-6) 
        int tm_yday;    Days since Jan. 1: 0-365 
        int tm_isdst;   +1 Daylight Savings Time, 0 No DST, * -1 don't know 
    };*/ 
    struct tm t = {0, 0, h, d, m-1, y-1900, 0, 0, -1};   
    return mktime(&t);
}

void printDateCriteria(struct DateCriteria* criteria){
    printf("%d %d %d %d\n", 
        criteria->year == NULL ? -1 : criteria->year->val1, 
        criteria->month == NULL ? -1 : criteria->month->val1, 
        criteria->day  == NULL ? -1 : criteria->day->val1, 
        criteria->hour == NULL ? -1 : criteria->hour->val1);    
}
void checkReplaceRelativeValues(int tsY, int tsM, int tsD, int tsH, 
        char* yTxt, char* mTxt, char* dTxt, char* hTxt, 
        int exY, int exM, int exD, int exH){
            
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, "*", hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    
    int resultOk = replaceRelativeValues(criteria, ts);
    //printDateCriteria(criteria);
    assert_true(resultOk);
    
    int yearOk  = ((criteria->year  == NULL) && (exY == -1)) || ((criteria->year  != NULL) && (criteria->year->val1  == exY));
    assert_true(yearOk);
    
    int monthOk = ((criteria->month == NULL) && (exM == -1)) || ((criteria->month != NULL) && (criteria->month->val1 == exM));
    assert_true(monthOk);
    
    int dayOk   = ((criteria->day   == NULL) && (exD == -1)) || ((criteria->day   != NULL) && (criteria->day->val1   == exD));
    assert_true(dayOk);
    
    int hourOk  = ((criteria->hour  == NULL) && (exH == -1)) || ((criteria->hour  != NULL) && (criteria->hour->val1  == exH));
    assert_true(hourOk);      
    
    freeDateCriteria(criteria);
}
int checkReplaceRelativeValuesInvalid(int tsY, int tsM, int tsD, int tsH, 
        char* yTxt, char* mTxt, char* dTxt, char* hTxt){
            
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, "*", hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    
    assert_int_equal(0, replaceRelativeValues(criteria, ts));
    freeDateCriteria(criteria);
}
void checkFirstMatchingDate(int tsY, int tsM, int tsD, int tsH, 
        char* yTxt, char* mTxt, char* wTxt, char* dTxt, char* hTxt, 
        int exY, int exM, int exD, int exH){
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, wTxt, hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    time_t result = findFirstMatchingDate(criteria, ts);
    
    struct tm* t = localtime(&result);
    assert_int_equal(exY, t->tm_year + 1900);
    assert_int_equal(exM, t->tm_mon + 1);
    assert_int_equal(exD, t->tm_mday);
    assert_int_equal(exH, t->tm_hour);
    freeDateCriteria(criteria);
}

void checkNoMatchingDates(int tsY, int tsM, int tsD, int tsH, char* yTxt, char* mTxt, char* wTxt, char* dTxt, char* hTxt){
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, wTxt, hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    time_t result = findFirstMatchingDate(criteria, ts);
    
    assert_int_equal(-1, result);
    freeDateCriteria(criteria);
}

void checkAlertTotal(struct Alert* alert, BW_INT vl){
    struct Data* data = getTotalsForAlert(alert, getTime());
    assert_int_equal(vl, data->vl);   
    freeData(data);
}

int callback(void* a, int b, char** c, char** d);
int count = 0;

void testRemoveAlertsDb(void** state){
    emptyDb();
    
    struct Alert* alert = allocAlert();
    alert->active = 1;
    alert->filter = 1;
    alert->amount = 100000000000;
    alert->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert->periods = period1;
    
    int alertId = addAlert(alert);
    assert_int_equal(1, getRowCount("SELECT * FROM alert;"));  
    assert_int_equal(2, getRowCount("SELECT * FROM alert_interval;")); 
    assert_int_equal(3, getRowCount("SELECT * FROM interval;"));
    
    removeAlert(alertId);
    assert_int_equal(0, getRowCount("SELECT * FROM alert;"));  
    assert_int_equal(0, getRowCount("SELECT * FROM alert_interval;")); 
    assert_int_equal(0, getRowCount("SELECT * FROM interval;"));  
    
    freeAlert(alert);
    freeStmtList();
}

void testAddGetRemoveAlerts(void** state){
    count = 0;
    struct Alert* alert = allocAlert();
    alert->id     = 2;
    alert->active = 1;
    alert->filter = 1;
    alert->amount = 100000000000;
    alert->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    
    alert->periods = period1;
    
    int alertId1 = addAlert(alert);
    
    freeAlert(alert);
    alert = allocAlert();
    alert->id      = 3;
    alert->active  = 0;
    alert->filter = 3;
    alert->amount    = 999999999999;
    alert->bound   = makeDateCriteria("2011", "6", "27", "5", "16");
    setAlertName(alert, "alert2");
    
    period1 = makeDateCriteria("*", "*", "*", "2,3,4", "1-7");
    period2 = makeDateCriteria("*", "*", "*", "1-5", "7-13");
    period1->next = period2;
    
    alert->periods = period1;
    
    int alertId2 = addAlert(alert);
    freeAlert(alert);
    
    struct Alert* firstAlert;
    struct Alert* resultAlert = getAlerts();
    firstAlert = resultAlert;
    assert_string_equal("alert1", resultAlert->name);
    assert_int_equal(alertId1, resultAlert->id);
    assert_int_equal(1, resultAlert->filter);
    assert_int_equal(100000000000, resultAlert->amount);
    assert_int_equal(1, resultAlert->active);
    checkDateCriteriaPart(resultAlert->bound->year,    0, 2010, 2010, 0);
    checkDateCriteriaPart(resultAlert->bound->month,   0, 5, 5, 0);
    checkDateCriteriaPart(resultAlert->bound->day,     0, 26, 26, 0);
    checkDateCriteriaPart(resultAlert->bound->weekday, 0, 4, 4, 0);
    checkDateCriteriaPart(resultAlert->bound->hour,    0, 15, 15, 0);
    
    struct DateCriteria* period = resultAlert->periods;
    assert_true(NULL == period->year);
    assert_true(NULL == period->month);
    assert_true(NULL == period->day);
    checkDateCriteriaPart(period->weekday, 0, 5, 5, 1);
    checkDateCriteriaPart(period->weekday->next, 0, 6, 6, 0);
    checkDateCriteriaPart(period->hour,    0, 0, 6, 0);
    
    period = period->next;
    assert_true(NULL == period->year);
    assert_true(NULL == period->month);
    assert_true(NULL == period->day);
    checkDateCriteriaPart(period->weekday, 0, 0, 4, 0);
    checkDateCriteriaPart(period->hour,    0, 6, 12, 0);
    
    resultAlert = resultAlert->next;
    
    assert_string_equal("alert2", resultAlert->name);
    assert_int_equal(alertId2, resultAlert->id);
    assert_int_equal(0, resultAlert->active);
    assert_int_equal(999999999999, resultAlert->amount);
    assert_int_equal(3, resultAlert->filter);
    checkDateCriteriaPart(resultAlert->bound->year,    0, 2011, 2011, 0);
    checkDateCriteriaPart(resultAlert->bound->month,   0, 6, 6, 0);
    checkDateCriteriaPart(resultAlert->bound->day,     0, 27, 27, 0);
    checkDateCriteriaPart(resultAlert->bound->weekday, 0, 5, 5, 0);
    checkDateCriteriaPart(resultAlert->bound->hour,    0, 16, 16, 0);
    
    period = resultAlert->periods;
    assert_true(NULL == period->year);
    assert_true(NULL == period->month);
    assert_true(NULL == period->day);
    checkDateCriteriaPart(period->weekday, 0, 2, 2, 1);
    checkDateCriteriaPart(period->weekday->next, 0, 3, 3, 1);
    checkDateCriteriaPart(period->weekday->next->next, 0, 4, 4, 0);
    checkDateCriteriaPart(period->hour,    0, 1, 7, 0);
    
    period = period->next;
    assert_true(NULL == period->year);
    assert_true(NULL == period->month);
    assert_true(NULL == period->day);
    checkDateCriteriaPart(period->weekday, 0, 1, 5, 0);
    checkDateCriteriaPart(period->hour,    0, 7, 13, 0);
    
    assert_true(NULL == resultAlert->next);
    
 // Test update of existing alert
    freeAlert(firstAlert);
    resultAlert = getAlerts();
    assert_int_equal(alertId1, resultAlert->id);
    resultAlert->amount = 1000;
    updateAlert(resultAlert);
    
    freeAlert(resultAlert);
    firstAlert = resultAlert = getAlerts();
    int found = FALSE;
    while(resultAlert != NULL){
        if (resultAlert->id == alertId1){
            assert_int_equal(1000, resultAlert->amount);
            found = TRUE;   
        }
        resultAlert = resultAlert->next;
    }
    assert_true(found);
    
    removeAlert(alertId1);
    freeAlert(firstAlert);
    
    resultAlert = getAlerts();
    assert_int_equal(alertId2, resultAlert->id);
    assert_true(NULL == resultAlert->next);
    removeAlert(alertId1);
    freeAlert(resultAlert);
    
    resultAlert = getAlerts();
    assert_int_equal(alertId2, resultAlert->id);
    assert_true(NULL == resultAlert->next);
    removeAlert(alertId2);
    freeAlert(resultAlert);
    
    resultAlert = getAlerts();
    assert_true(NULL == resultAlert);
    freeAlert(resultAlert);
    freeStmtList();
}
void testIsDateCriteriaPartMatch(void** state){
    assert_int_equal(1, isDateCriteriaPartMatch(NULL, 0));
    assert_int_equal(1, isDateCriteriaPartMatch(NULL, 1));
    assert_int_equal(1, isDateCriteriaPartMatch(NULL, 12));
    
    struct DateCriteriaPart part2 = {0,1,1,NULL};
    assert_int_equal(1, isDateCriteriaPartMatch(&part2, 1));
    assert_int_equal(0, isDateCriteriaPartMatch(&part2, 0));
    assert_int_equal(0, isDateCriteriaPartMatch(&part2, 2));
    assert_int_equal(0, isDateCriteriaPartMatch(&part2, 12));
    
    struct DateCriteriaPart part3 = {0,1,3,NULL};
    assert_int_equal(0, isDateCriteriaPartMatch(&part3, 0));
    assert_int_equal(1, isDateCriteriaPartMatch(&part3, 1));
    assert_int_equal(1, isDateCriteriaPartMatch(&part3, 2));
    assert_int_equal(1, isDateCriteriaPartMatch(&part3, 3));
    assert_int_equal(0, isDateCriteriaPartMatch(&part3, 4));
    
    struct DateCriteriaPart part4 = {0,1,1,NULL};
    struct DateCriteriaPart part5 = {0,4,5,&part4};
    assert_int_equal(0, isDateCriteriaPartMatch(&part5, 0));
    assert_int_equal(1, isDateCriteriaPartMatch(&part5, 1));
    assert_int_equal(0, isDateCriteriaPartMatch(&part5, 2));
    assert_int_equal(0, isDateCriteriaPartMatch(&part5, 3));
    assert_int_equal(1, isDateCriteriaPartMatch(&part5, 4));
    assert_int_equal(1, isDateCriteriaPartMatch(&part5, 5));
    assert_int_equal(0, isDateCriteriaPartMatch(&part5, 6));
}
void testIsDateCriteriaMatch(void** state){
    struct DateCriteria* criteria1 = makeDateCriteria("*", "1,4-5,11", "1-20", "*", "5");
    
    assert_int_equal(0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 4)));
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 5)));
    assert_int_equal(0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 6)));
    
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5,  1, 5)));
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 5)));
    assert_int_equal(0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 21, 5)));
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 1, 20, 5)));
    assert_int_equal(0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 2, 20, 5)));
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 5)));
    assert_int_equal(0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 7, 20, 5)));
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 11, 20, 5)));
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(2011, 5, 20, 5)));
    assert_int_equal(1, isDateCriteriaMatch(criteria1, makeTsFromParts(1980, 5, 20, 5)));
    freeDateCriteria(criteria1);    
    
    struct DateCriteria* criteria2 = makeDateCriteria("*", "4,5", "*", "4", "*");
    assert_int_equal(0, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5,  5, 0)));
    assert_int_equal(1, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5,  6, 0)));
    assert_int_equal(0, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5,  7, 0)));
    assert_int_equal(1, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5, 13, 0)));
    assert_int_equal(1, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5, 20, 0)));
    freeDateCriteria(criteria2);
    
    struct DateCriteria* criteria3 = makeDateCriteria("2010", "*", "*", "*", "*");
    assert_int_equal(0, isDateCriteriaMatch(criteria3, makeTsFromParts(2009, 12, 31, 23)));
    assert_int_equal(1, isDateCriteriaMatch(criteria3, makeTsFromParts(2010, 1,  1, 0)));
    assert_int_equal(1, isDateCriteriaMatch(criteria3, makeTsFromParts(2010, 12,  31, 23)));
    assert_int_equal(0, isDateCriteriaMatch(criteria3, makeTsFromParts(2011, 1,  1, 0)));
    freeDateCriteria(criteria3);
}
void testFindLowestMatch(void** state){    
    struct DateCriteriaPart part6 = {0,2,4,NULL};
    assert_int_equal(2, findLowestMatch(&part6));
    
    struct DateCriteriaPart part7 = {0,2,4,NULL};
    struct DateCriteriaPart part8 = {0,10,10,&part7};
    struct DateCriteriaPart part9 = {0,5,6,&part8};
    assert_int_equal(2, findLowestMatch(&part9));
}
void testFindHighestMatchAtOrBelowLimit(void** state){        
    struct DateCriteriaPart part10 = {0,5,6,NULL};
    assert_int_equal( 6, findHighestMatchAtOrBelowLimit(&part10, 10));
    assert_int_equal( 6, findHighestMatchAtOrBelowLimit(&part10, 6));
    assert_int_equal( 5, findHighestMatchAtOrBelowLimit(&part10, 5));
    assert_int_equal(-1, findHighestMatchAtOrBelowLimit(&part10, 4));
    
    struct DateCriteriaPart part11 = {0,2,4,NULL};
    struct DateCriteriaPart part12 = {0,10,10,&part11};
    struct DateCriteriaPart part13 = {0,5,6,&part12};
    assert_int_equal(10, findHighestMatchAtOrBelowLimit(&part13, 11));
    assert_int_equal(10, findHighestMatchAtOrBelowLimit(&part13, 10));
    assert_int_equal(6,  findHighestMatchAtOrBelowLimit(&part13, 9));
    assert_int_equal(6,  findHighestMatchAtOrBelowLimit(&part13, 8));
    assert_int_equal(6,  findHighestMatchAtOrBelowLimit(&part13, 7));
    assert_int_equal(6,  findHighestMatchAtOrBelowLimit(&part13, 6));
    assert_int_equal(5,  findHighestMatchAtOrBelowLimit(&part13, 5));
    assert_int_equal(4,  findHighestMatchAtOrBelowLimit(&part13, 4));
    assert_int_equal(3,  findHighestMatchAtOrBelowLimit(&part13, 3));
    assert_int_equal(2,  findHighestMatchAtOrBelowLimit(&part13, 2));
    assert_int_equal(-1, findHighestMatchAtOrBelowLimit(&part13, 1));
}
void testFindHighestMatch(void** state){         
    struct DateCriteriaPart part14 = {0,2,4,NULL};
    assert_int_equal(4, findHighestMatch(&part14));
    
    struct DateCriteriaPart part15 = {0,2,4,NULL};
    struct DateCriteriaPart part16 = {0,10,10,&part15};
    struct DateCriteriaPart part17 = {0,5,6,&part16};
    assert_int_equal(10, findHighestMatch(&part17));
}
void testGetNonRelativeValue(void** state){        
    struct DateCriteriaPart* nonRelPart = getNonRelativeValue(6);                                           
    assert_int_equal(6, nonRelPart->val1);                                                                    
    assert_int_equal(6, nonRelPart->val2);                                                                    
    assert_int_equal(FALSE, nonRelPart->isRelative);                                                          
    assert_true(NULL == nonRelPart->next);       
    freeDateCriteriaPart(nonRelPart);
}
static int replaceRelativeValuesAndFree(struct DateCriteria* criteria, time_t ts){
    int val = replaceRelativeValues(criteria, ts);
    freeDateCriteria(criteria);
    return val;
}
void testReplaceRelativeValues(void** state){
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "*", "*", "-0", -1, -1, -1,  3);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "*", "*", "-1", -1, -1, -1,  2);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "*", "*", "-5", 2010, 5, 25, 22);
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "1", "*", "*", "-1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "1", "*", "-1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "*", "1", "-1");
    
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "*",  "-0",  "0", -1, -1, 26, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "*",  "-1",  "0", -1, -1, 25, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "*", "-30",  "0", 2010, 4, 26, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "*",  "-1", "-0", -1, -1, 25, 3);
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "*", "-1", "-1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "*", "-1",  "*");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "1", "-1",  "0");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "1", "*", "-1",  "0");
    
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "-0",   "1",  "0", -1,  5,  1, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "-1",   "1",  "0", -1,  4,  1, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "-6",   "1",  "0", 2009, 11,  1, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "*", "-6",  "-0", "-0", 2009, 11, 26, 3);
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "-1",  "1", "-1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "-1", "-1",  "0");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "-1",  "1",  "*");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "*", "-1",  "*",  "0");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "1", "-1",  "1",  "0");
    
    checkReplaceRelativeValues(2010, 5, 26, 3, "-0",  "1",  "1",  "0", 2010,  1,  1, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "-1",  "1",  "1",  "0", 2009,  1,  1, 0);
    checkReplaceRelativeValues(2010, 5, 26, 3, "-1", "-0", "-0", "-0", 2009,  5, 26, 3);
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "-1",  "*",   "1",  "1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "-1",  "1",   "*",  "1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "-1",  "1",   "1",  "*");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "-1", "-1",   "1",  "1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "-1",  "1",  "-1",  "1");
    checkReplaceRelativeValuesInvalid(2010, 5, 26, 3, "-1",  "1",   "1", "-1");
    
    struct tm t1 = {0, 0, 3, 26, 4, 72, 0, 0, -1};
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("1990",  "*",  "*", "*", "-1"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("*",    "12",  "*", "*", "-1"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("*",     "*", "31", "*", "-1"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("2001",  "*", "-1", "*",  "1"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("*",    "12", "-1", "*",  "1"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("2002", "-1",  "*", "*",  "1"), mktime(&t1))); 
    
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("-1",  "1",  "1", "*", "*"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("-1",  "1",  "*", "*", "0"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("-1",  "*",  "1", "*", "0"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("*",  "-1",  "1", "*", "*"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("*",  "-1",  "*", "*", "0"), mktime(&t1))); 
    assert_int_equal(0, replaceRelativeValuesAndFree(makeDateCriteria("*",  "*",  "-1", "*", "*"), mktime(&t1))); 
}
void testFindFirstMatchingDate(void** state){    
    checkFirstMatchingDate(2010, 5, 12, 0, "*", "*", "*", "*", "*", 2010, 5, 12, 0); 
    
    checkFirstMatchingDate(2010,  5, 12,  0,      "2010", "*", "*", "*", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(2010,  5, 12,  3,        "-0", "1", "*", "1", "0", 2010,  1,  1,  0); 
    checkFirstMatchingDate(2010,  5, 12,  0, "2000-2020", "*", "*", "*", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(2010,  5, 12,  9, "2000,2010", "*", "*", "*", "*", 2010,  5, 12,  9); 
    checkFirstMatchingDate(2010, 11,  1,  0, "2010,2011", "*", "*", "*", "*", 2010, 11,  1,  0); 
    checkFirstMatchingDate(2010,  5, 12,  3,      "2009", "*", "*", "*", "*", 2009, 12, 31, 23); 
    checkFirstMatchingDate(2020,  1,  1,  6,      "2007", "*", "*", "*", "*", 2007, 12, 31, 23); 
    checkFirstMatchingDate(2005,  1,  1,  0,      "2005", "*", "*", "*", "*", 2005,  1,  1,  0); 
    checkFirstMatchingDate(2005, 12, 31,  0,      "2005", "*", "*", "*", "*", 2005, 12, 31,  0); 
    checkFirstMatchingDate(2005, 12, 31, 23,      "2005", "*", "*", "*", "*", 2005, 12, 31, 23); 
    checkFirstMatchingDate(2005, 12, 31, 23,        "-7", "1", "*", "1", "0", 1998,  1,  1,  0); 
    
    checkNoMatchingDates(2010, 5, 12, 0, "2011",           "*", "*", "*", "*");
    checkNoMatchingDates(2010, 5, 12, 0, "2011,2015-2020", "*", "*", "*", "*");
    
    checkFirstMatchingDate(2010,  5, 12,  0, "*",       "5", "*", "*", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(2010,  5, 12,  0, "*",       "4", "*", "*", "*", 2010,  4, 30, 23); 
    checkFirstMatchingDate(2010,  5, 12,  0, "*",       "9", "*", "*", "*", 2009,  9, 30, 23); 
    checkFirstMatchingDate(2010,  5, 12,  0, "*", "1,3-4,9", "*", "*", "*", 2010,  4, 30, 23); 
    checkFirstMatchingDate(2010,  5, 12,  0, "*",      "-2", "*", "1", "0", 2010,  3,  1,  0);       
    checkFirstMatchingDate(2010,  5, 12,  0, "*",     "-12", "*", "1", "1", 2009,  5,  1,  1); 
    checkFirstMatchingDate(2010,  5, 12,  1, "*",      "-0", "*", "1", "0", 2010,  5,  1,  0); 
    checkFirstMatchingDate(2000,  1,  1,  0, "*",      "-0", "*", "1", "0", 2000,  1,  1,  0); 
    checkFirstMatchingDate(2000, 12, 31, 23, "*",      "-0", "*", "1", "0", 2000, 12,  1,  0); 
                                                                                               
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "2",  "*", "*", 2010,  5, 11, 16); 
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "1",  "*", "*", 2010,  5, 10, 23); 
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "0",  "*", "*", 2010,  5,  9, 23); 
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "6",  "*", "*", 2010,  5,  8, 23); 
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "5",  "*", "*", 2010,  5,  7, 23); 
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "4",  "*", "*", 2010,  5,  6, 23); 
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "3",  "*", "*", 2010,  5,  5, 23); 
    checkFirstMatchingDate(2010, 5, 12, 16, "*", "*", "1,2",  "*", "*", 2010,  5, 11, 23); 
    checkFirstMatchingDate(2010, 5, 12, 16, "*", "*", "4-6",  "*", "*", 2010,  5,  8, 23); 
    checkFirstMatchingDate(2010, 5,  1,  0, "*", "*",   "6",  "*", "*", 2010,  5,  1,  0); 
    checkFirstMatchingDate(2010, 5,  1,  0, "*", "*",   "5",  "*", "*", 2010,  4, 30, 23); 
    checkFirstMatchingDate(2010, 1,  1,  0, "*", "*",   "5",  "*", "*", 2010,  1,  1,  0);
    checkFirstMatchingDate(2010, 1,  1,  0, "*", "*",   "4",  "*", "*", 2009, 12, 31, 23);
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "4",  "1", "*", 2010,  4,  1, 23);
    checkFirstMatchingDate(2010, 5, 11, 16, "*", "*",   "2", "30", "2", 2010,  3, 30,  2);
    
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",     "12", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",     "19", "*", 2010,  4, 19, 23); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",      "7", "*", 2010,  5,  7, 23); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",     "-2", "0", 2010,  5, 10,  0); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",    "-12", "0", 2010,  4, 30,  0); 
    checkFirstMatchingDate(2020, 1,  7, 14, "*", "*", "*",     "-8", "0", 2019, 12, 30,  0); 
    checkFirstMatchingDate(2020, 1,  1,  0, "*", "*", "*",     "-0", "0", 2020,  1,  1,  0); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",  "1,2,3", "*", 2010,  5,  3, 23); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",    "1-5", "*", 2010,  5,  5, 23); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",   "1-12", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*",  "20-25", "*", 2010,  4, 25, 23); 
    
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*", "*",    "-0", 2010,  5, 12,  0); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*", "*",    "-1", 2010,  5, 11, 23); 
    checkFirstMatchingDate(2010, 5, 12,  0, "*", "*", "*", "*",    "-1", 2010,  5, 11, 23); 
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*", "*",    "15", 2010,  5, 12, 15); 
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*", "*",    "12", 2010,  5, 12, 12); 
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*", "*", "1,2-7", 2010,  5, 12,  7); 
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*", "*",    "16", 2010,  5, 11, 16); 
    checkFirstMatchingDate(2010, 1,  1,  0, "*", "*", "*", "*",    "16", 2009, 12, 31, 16); 
    
    checkFirstMatchingDate(2010, 5, 12, 15, "2010", "5", "*",  "*",  "*", 2010, 5, 12, 15); 
    checkFirstMatchingDate(2010, 5, 12, 15, "2008", "5", "*",  "*",  "*", 2008, 5, 31, 23); 
    checkFirstMatchingDate(2010, 5, 12, 15, "2008", "5", "*", "31",  "*", 2008, 5, 31, 23); 
    checkFirstMatchingDate(2010, 5, 12, 15, "2008", "5", "*", "31", "23", 2008, 5, 31, 23); 
    checkNoMatchingDates(2010, 5, 12,  0, "2010", "6", "*",  "*",     "*");
    checkNoMatchingDates(2010, 5, 12, 15, "2010", "5", "*", "31",     "*"); 
    checkNoMatchingDates(2010, 5, 12, 15, "2010", "5", "*", "12", "16-23"); 
    
    checkFirstMatchingDate(2010, 5, 12, 15, "-1", "1", "*", "1", "0", 2009, 1,  1,  0); 
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "-1", "*", "1", "0", 2010,  4,  1,  0); 
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "-6", "*", "1", "0", 2009, 11,  1,  0); 
    checkFirstMatchingDate(2010, 5, 12, 15, "2009", "11", "*", "*", "0", 2009, 11, 30,  0);
    
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "-6", "*", "-0", "-0", 2009, 11, 12, 15);
    
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*",  "-1", "0", 2010, 5, 11,  0);
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*", "-15", "0", 2010, 4, 27,  0);
    
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*",  "*",  "-1", 2010, 5, 12, 14);
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*",  "*", "-20", 2010, 5, 11, 19);
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*",  "*", "-24", 2010, 5, 11, 15);
    
 // Common scenarios
    
 // First day of month
    checkFirstMatchingDate(2010, 5,  1,  0, "*", "*", "*", "1", "0", 2010, 5, 1, 0);
    checkFirstMatchingDate(2010, 5, 12, 15, "*", "*", "*", "1", "0", 2010, 5, 1, 0);
    checkFirstMatchingDate(2010, 5, 31, 23, "*", "*", "*", "1", "0", 2010, 5, 1, 0);
    checkFirstMatchingDate(2010, 6,  1,  0, "*", "*", "*", "1", "0", 2010, 6, 1, 0);
    
 // Rolling 30 day period
    checkFirstMatchingDate(2010, 5,  1,  0, "*", "*", "*", "-30", "-0", 2010, 4,  1,  0);
    checkFirstMatchingDate(2010, 5, 10,  3, "*", "*", "*", "-30", "-0", 2010, 4, 10,  3);
    checkFirstMatchingDate(2010, 5, 31, 23, "*", "*", "*", "-30", "-0", 2010, 5,  1, 23);
    
 // Since Sunday
    checkFirstMatchingDate(2010, 5, 11,  0, "*", "*", "0", "*", "*", 2010, 5, 9, 23);
    checkFirstMatchingDate(2010, 5, 10,  0, "*", "*", "0", "*", "*", 2010, 5, 9, 23);
    checkFirstMatchingDate(2010, 5,  9,  2, "*", "*", "0", "*", "*", 2010, 5, 9,  2);
    checkFirstMatchingDate(2010, 5,  8, 23, "*", "*", "0", "*", "*", 2010, 5, 2, 23);
}
void testGetTotalsForAlert(void** state){
    emptyDb();
    setTime(makeTs("2010-05-30 12:00:00"));
    
    struct Alert* alert = allocAlert();
    alert->id     = 1;
    alert->active = 1;
    alert->filter = 1;
    alert->amount = 10000;
    alert->bound  = makeDateCriteria("2010", "5", "1", "*", "0");
    setAlertName(alert, "alert1");
    checkAlertTotal(alert, 0);
    
    addDbRow(makeTs("2010-04-30 23:00:00"), 1, 1, 1);
    checkAlertTotal(alert, 0);
    
    addDbRow(makeTs("2010-05-01 00:00:00"), 1, 2, 1);
    checkAlertTotal(alert, 0);
    
    addDbRow(makeTs("2010-05-01 01:00:00"), 1, 4, 1);
    checkAlertTotal(alert, 4);
    
    addDbRow(makeTs("2010-05-10 22:00:00"), 1, 8, 1);
    checkAlertTotal(alert, 12);
    
    addDbRow(makeTs("2010-05-10 23:00:00"), 1, 16, 1);
    checkAlertTotal(alert, 28);
    
    alert->filter = 2;
    checkAlertTotal(alert, 0);
    
    emptyDb();
    
    addDbRow(makeTs("2010-05-01 12:00:00"), 3600, 1, 1); // Saturday (no)
    addDbRow(makeTs("2010-05-01 13:00:00"), 3600, 2, 1); // Saturday (yes)
    addDbRow(makeTs("2010-05-01 16:00:00"), 3600, 4, 1); // Saturday (yes)
    addDbRow(makeTs("2010-05-01 17:00:00"), 3600, 8, 1); // Saturday (yes) '12-16' = 12:**-16:**
    addDbRow(makeTs("2010-05-01 18:00:00"), 3600, 1, 1); // Saturday (no)
    
    addDbRow(makeTs("2010-05-02 12:00:00"), 3600,  1, 1); // Sunday (no)
    addDbRow(makeTs("2010-05-02 13:00:00"), 3600, 16, 1); // Sunday (yes)
    addDbRow(makeTs("2010-05-02 16:00:00"), 3600, 32, 1); // Sunday (yes)
    addDbRow(makeTs("2010-05-02 17:00:00"), 3600, 64, 1); // Sunday (yes) '12-16' = 12:**-16:**
    addDbRow(makeTs("2010-05-02 18:00:00"), 3600,  1, 1); // Sunday (no)
    
    addDbRow(makeTs("2010-05-03 06:00:00"), 3600,    1, 1); // Monday (no)
    addDbRow(makeTs("2010-05-03 07:00:00"), 3600,  128, 1); // Monday (yes)
    addDbRow(makeTs("2010-05-03 12:00:00"), 3600,  256, 1); // Monday (yes)
    addDbRow(makeTs("2010-05-03 19:00:00"), 3600, 1024, 1); // Monday (yes) '6-18' = 06:**-18:**
    addDbRow(makeTs("2010-05-03 20:00:00"), 3600,    1, 1); // Monday (no)
    
    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "1-5", "6-18");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "0,6", "12-16");
    period3->next = period4;
    alert->periods = period3;
    
    alert->filter = 1;
    checkAlertTotal(alert, 1534);     
    alert->filter = 2;
    checkAlertTotal(alert, 0);      
    
    freeAlert(alert);
    freeStmtList();
}

int callback(void* a, int b, char** c, char** d){
    count++;
}

