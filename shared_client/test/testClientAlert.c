/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "client.h"
#include "CuTest.h"
#include "test.h"

static time_t makeTsFromParts(int y, int m, int d, int h){
 /*struct tm {
    	int	tm_sec;		Seconds: 0-59 (K&R says 0-61?) 
    	int	tm_min;		Minutes: 0-59 
    	int	tm_hour;	Hours since midnight: 0-23 
    	int	tm_mday;	Day of the month: 1-31 
    	int	tm_mon;		Months *since* january: 0-11 
    	int	tm_year;	Years since 1900 
    	int	tm_wday;	Days since Sunday (0-6) 
    	int	tm_yday;	Days since Jan. 1: 0-365 
    	int	tm_isdst;	+1 Daylight Savings Time, 0 No DST, * -1 don't know 
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
void checkReplaceRelativeValues(CuTest *tc, int tsY, int tsM, int tsD, int tsH, 
        char* yTxt, char* mTxt, char* dTxt, char* hTxt, 
        int exY, int exM, int exD, int exH){
            
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, "*", hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    
    int resultOk = replaceRelativeValues(criteria, ts);
	printDateCriteria(criteria);
    CuAssertTrue(tc, resultOk);
	
    int yearOk  = ((criteria->year  == NULL) && (exY == -1)) || ((criteria->year  != NULL) && (criteria->year->val1  == exY));
    CuAssertTrue(tc, yearOk);
    
    int monthOk = ((criteria->month == NULL) && (exM == -1)) || ((criteria->month != NULL) && (criteria->month->val1 == exM));
    CuAssertTrue(tc, monthOk);
    
    int dayOk   = ((criteria->day   == NULL) && (exD == -1)) || ((criteria->day   != NULL) && (criteria->day->val1   == exD));
    CuAssertTrue(tc, dayOk);
    
    int hourOk  = ((criteria->hour  == NULL) && (exH == -1)) || ((criteria->hour  != NULL) && (criteria->hour->val1  == exH));
    CuAssertTrue(tc, hourOk);    
}
int checkReplaceRelativeValuesInvalid(CuTest *tc, int tsY, int tsM, int tsD, int tsH, 
        char* yTxt, char* mTxt, char* dTxt, char* hTxt){
            
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, "*", hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    
    CuAssertIntEquals(tc, 0, replaceRelativeValues(criteria, ts));
}
void checkFirstMatchingDate(CuTest *tc, int tsY, int tsM, int tsD, int tsH, 
        char* yTxt, char* mTxt, char* wTxt, char* dTxt, char* hTxt, 
        int exY, int exM, int exD, int exH){
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, wTxt, hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    time_t result = findFirstMatchingDate(criteria, ts);
    
    struct tm* t = localtime(&result);
    CuAssertIntEquals(tc, exY, t->tm_year + 1900);
    CuAssertIntEquals(tc, exM, t->tm_mon + 1);
    CuAssertIntEquals(tc, exD, t->tm_mday);
    CuAssertIntEquals(tc, exH, t->tm_hour);
}

void checkNoMatchingDates(CuTest *tc, int tsY, int tsM, int tsD, int tsH, char* yTxt, char* mTxt, char* wTxt, char* dTxt, char* hTxt){
    struct DateCriteria* criteria = makeDateCriteria(yTxt, mTxt, dTxt, wTxt, hTxt);
    time_t ts = makeTsFromParts(tsY, tsM, tsD, tsH);
    time_t result = findFirstMatchingDate(criteria, ts);
    
    CuAssertIntEquals(tc, -1, result);
}

void checkAlertTotals(CuTest *tc, struct Alert* alert, BW_INT dl, BW_INT ul){
    struct Data* data = getTotalsForAlert(alert);
    CuAssertIntEquals(tc, dl, data->dl);
    CuAssertIntEquals(tc, ul, data->ul);   
}

int callback(void* a, int b, char** c, char** d);
int count = 0;

void testRemoveAlertsDb(CuTest *tc){
	emptyDb();
	
	struct Alert* alert = allocAlert();
    alert->active    = 1;
    alert->direction = 1;
    alert->amount    = 100000000000;
    alert->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert->periods = period1;
    
    int alertId = addAlert(alert);
    CuAssertIntEquals(tc, 1, getRowCount("SELECT * FROM alert;"));  
    CuAssertIntEquals(tc, 2, getRowCount("SELECT * FROM alert_interval;")); 
    CuAssertIntEquals(tc, 3, getRowCount("SELECT * FROM interval;"));
    
    removeAlert(alertId);
    CuAssertIntEquals(tc, 0, getRowCount("SELECT * FROM alert;"));  
    CuAssertIntEquals(tc, 0, getRowCount("SELECT * FROM alert_interval;")); 
    //CuAssertIntEquals(tc, 0, getRowCount("SELECT * FROM interval;"));
}

void testAddGetRemoveAlerts(CuTest *tc){
    count = 0;
    struct Alert* alert = allocAlert();
    alert->id        = 2;
    alert->active    = 1;
    alert->direction = 1;
    alert->amount    = 100000000000;
    alert->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    
    alert->periods = period1;
    
    int alertId1 = addAlert(alert);
    
    alert = allocAlert();
    alert->id      = 3;
    alert->active  = 0;
    alert->direction = 3;
    alert->amount    = 999999999999;
    alert->bound   = makeDateCriteria("2011", "6", "27", "5", "16");
    setAlertName(alert, "alert2");
    
    period1 = makeDateCriteria("*", "*", "*", "2,3,4", "1-7");
    period2 = makeDateCriteria("*", "*", "*", "1-5", "7-13");
    period1->next = period2;
    
    alert->periods = period1;
    
    int alertId2 = addAlert(alert);
    
    struct Alert* resultAlert = getAlerts();
    CuAssertStrEquals(tc,"alert1", resultAlert->name);
    CuAssertIntEquals(tc, alertId1, resultAlert->id);
    CuAssertIntEquals(tc, 1, resultAlert->direction);
    CuAssertIntEquals(tc, 100000000000, resultAlert->amount);
    CuAssertIntEquals(tc, 1, resultAlert->active);
    checkDateCriteriaPart(tc, resultAlert->bound->year,    0, 2010, 2010, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->month,   0, 5, 5, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->day,     0, 26, 26, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->weekday, 0, 4, 4, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->hour,    0, 15, 15, 0);
    
    struct DateCriteria* period = resultAlert->periods;
    CuAssertTrue(tc, NULL == period->year);
    CuAssertTrue(tc, NULL == period->month);
    CuAssertTrue(tc, NULL == period->day);
    checkDateCriteriaPart(tc, period->weekday, 0, 5, 5, 1);
    checkDateCriteriaPart(tc, period->weekday->next, 0, 6, 6, 0);
    checkDateCriteriaPart(tc, period->hour,    0, 0, 6, 0);
    
    period = period->next;
    CuAssertTrue(tc, NULL == period->year);
    CuAssertTrue(tc, NULL == period->month);
    CuAssertTrue(tc, NULL == period->day);
    checkDateCriteriaPart(tc, period->weekday, 0, 0, 4, 0);
    checkDateCriteriaPart(tc, period->hour,    0, 6, 12, 0);
    
    resultAlert = resultAlert->next;
    
    CuAssertStrEquals(tc,"alert2", resultAlert->name);
    CuAssertIntEquals(tc, alertId2, resultAlert->id);
    CuAssertIntEquals(tc, 0, resultAlert->active);
    CuAssertIntEquals(tc, 999999999999, resultAlert->amount);
    CuAssertIntEquals(tc, 3, resultAlert->direction);
    checkDateCriteriaPart(tc, resultAlert->bound->year,    0, 2011, 2011, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->month,   0, 6, 6, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->day,     0, 27, 27, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->weekday, 0, 5, 5, 0);
    checkDateCriteriaPart(tc, resultAlert->bound->hour,    0, 16, 16, 0);
    
    period = resultAlert->periods;
    CuAssertTrue(tc, NULL == period->year);
    CuAssertTrue(tc, NULL == period->month);
    CuAssertTrue(tc, NULL == period->day);
    checkDateCriteriaPart(tc, period->weekday, 0, 2, 2, 1);
    checkDateCriteriaPart(tc, period->weekday->next, 0, 3, 3, 1);
    checkDateCriteriaPart(tc, period->weekday->next->next, 0, 4, 4, 0);
    checkDateCriteriaPart(tc, period->hour,    0, 1, 7, 0);
    
    period = period->next;
    CuAssertTrue(tc, NULL == period->year);
    CuAssertTrue(tc, NULL == period->month);
    CuAssertTrue(tc, NULL == period->day);
    checkDateCriteriaPart(tc, period->weekday, 0, 1, 5, 0);
    checkDateCriteriaPart(tc, period->hour,    0, 7, 13, 0);
    
    CuAssertTrue(tc, NULL == resultAlert->next);
    
 // Test update of existing alert
    resultAlert = getAlerts();
    CuAssertIntEquals(tc, alertId1, resultAlert->id);
    resultAlert->amount = 1000;
    updateAlert(resultAlert);
    
    resultAlert = getAlerts();
    int found = FALSE;
    while(resultAlert != NULL){
    	if (resultAlert->id == alertId1){
    		CuAssertIntEquals(tc, 1000, resultAlert->amount);
    		found = TRUE;	
    	}
    	resultAlert = resultAlert->next;
    }
    CuAssertTrue(tc, found);
    
    removeAlert(alertId1);
    resultAlert = getAlerts();
    CuAssertIntEquals(tc, alertId2, resultAlert->id);
    CuAssertTrue(tc, NULL == resultAlert->next);
    
    removeAlert(alertId1);
    resultAlert = getAlerts();
    CuAssertIntEquals(tc, alertId2, resultAlert->id);
    CuAssertTrue(tc, NULL == resultAlert->next);
    
    removeAlert(alertId2);
    resultAlert = getAlerts();
    CuAssertTrue(tc, NULL == resultAlert);
    

}
void testIsDateCriteriaPartMatch(CuTest* tc){
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(NULL, 0));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(NULL, 1));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(NULL, 12));
    
    struct DateCriteriaPart part2 = {0,1,1,NULL};
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(&part2, 1));
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part2, 0));
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part2, 2));
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part2, 12));

    struct DateCriteriaPart part3 = {0,1,3,NULL};
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part3, 0));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(&part3, 1));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(&part3, 2));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(&part3, 3));
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part3, 4));

    struct DateCriteriaPart part4 = {0,1,1,NULL};
    struct DateCriteriaPart part5 = {0,4,5,&part4};
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part5, 0));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(&part5, 1));
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part5, 2));
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part5, 3));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(&part5, 4));
    CuAssertIntEquals(tc, 1, isDateCriteriaPartMatch(&part5, 5));
    CuAssertIntEquals(tc, 0, isDateCriteriaPartMatch(&part5, 6));
}
void testIsDateCriteriaMatch(CuTest* tc){
    struct DateCriteria* criteria1 = makeDateCriteria("*", "1,4-5,11", "1-20", "*", "5");
    
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 4)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 5)));
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 6)));
    
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5,  1, 5)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 5)));
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 21, 5)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 1, 20, 5)));
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 2, 20, 5)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 5, 20, 5)));
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 7, 20, 5)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(2010, 11, 20, 5)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(2011, 5, 20, 5)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria1, makeTsFromParts(1980, 5, 20, 5)));

    struct DateCriteria* criteria2 = makeDateCriteria("*", "4,5", "*", "4", "*");
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5,  5, 0)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5,  6, 0)));
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5,  7, 0)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5, 13, 0)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria2, makeTsFromParts(2010, 5, 20, 0)));

    struct DateCriteria* criteria3 = makeDateCriteria("2010", "*", "*", "*", "*");
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria3, makeTsFromParts(2009, 12, 31, 23)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria3, makeTsFromParts(2010, 1,  1, 0)));
    CuAssertIntEquals(tc, 1, isDateCriteriaMatch(criteria3, makeTsFromParts(2010, 12,  31, 23)));
    CuAssertIntEquals(tc, 0, isDateCriteriaMatch(criteria3, makeTsFromParts(2011, 1,  1, 0)));
}
void testFindLowestMatch(CuTest* tc){    
    struct DateCriteriaPart part6 = {0,2,4,NULL};
    CuAssertIntEquals(tc, 2, findLowestMatch(&part6));
    
    struct DateCriteriaPart part7 = {0,2,4,NULL};
    struct DateCriteriaPart part8 = {0,10,10,&part7};
    struct DateCriteriaPart part9 = {0,5,6,&part8};
    CuAssertIntEquals(tc, 2, findLowestMatch(&part9));
}
void testFindHighestMatchAtOrBelowLimit(CuTest* tc){        
    struct DateCriteriaPart part10 = {0,5,6,NULL};
    CuAssertIntEquals(tc,  6, findHighestMatchAtOrBelowLimit(&part10, 10));
    CuAssertIntEquals(tc,  6, findHighestMatchAtOrBelowLimit(&part10, 6));
    CuAssertIntEquals(tc,  5, findHighestMatchAtOrBelowLimit(&part10, 5));
    CuAssertIntEquals(tc, -1, findHighestMatchAtOrBelowLimit(&part10, 4));
    
    struct DateCriteriaPart part11 = {0,2,4,NULL};
    struct DateCriteriaPart part12 = {0,10,10,&part11};
    struct DateCriteriaPart part13 = {0,5,6,&part12};
    CuAssertIntEquals(tc, 10, findHighestMatchAtOrBelowLimit(&part13, 11));
    CuAssertIntEquals(tc, 10, findHighestMatchAtOrBelowLimit(&part13, 10));
    CuAssertIntEquals(tc, 6,  findHighestMatchAtOrBelowLimit(&part13, 9));
    CuAssertIntEquals(tc, 6,  findHighestMatchAtOrBelowLimit(&part13, 8));
    CuAssertIntEquals(tc, 6,  findHighestMatchAtOrBelowLimit(&part13, 7));
    CuAssertIntEquals(tc, 6,  findHighestMatchAtOrBelowLimit(&part13, 6));
    CuAssertIntEquals(tc, 5,  findHighestMatchAtOrBelowLimit(&part13, 5));
    CuAssertIntEquals(tc, 4,  findHighestMatchAtOrBelowLimit(&part13, 4));
    CuAssertIntEquals(tc, 3,  findHighestMatchAtOrBelowLimit(&part13, 3));
    CuAssertIntEquals(tc, 2,  findHighestMatchAtOrBelowLimit(&part13, 2));
    CuAssertIntEquals(tc, -1, findHighestMatchAtOrBelowLimit(&part13, 1));
}
void testFindHighestMatch(CuTest* tc){         
    struct DateCriteriaPart part14 = {0,2,4,NULL};
    CuAssertIntEquals(tc, 4, findHighestMatch(&part14));
    
    struct DateCriteriaPart part15 = {0,2,4,NULL};
    struct DateCriteriaPart part16 = {0,10,10,&part15};
    struct DateCriteriaPart part17 = {0,5,6,&part16};
    CuAssertIntEquals(tc, 10, findHighestMatch(&part17));
}
void testGetNonRelativeValue(CuTest* tc){        
    struct DateCriteriaPart* nonRelPart = getNonRelativeValue(6);                                           
    CuAssertIntEquals(tc, 6, nonRelPart->val1);                                                                    
    CuAssertIntEquals(tc, 6, nonRelPart->val2);                                                                    
    CuAssertIntEquals(tc, FALSE, nonRelPart->isRelative);                                                          
    CuAssertTrue(tc, NULL == nonRelPart->next);
}
void testReplaceRelativeValues(CuTest* tc){
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "*", "*", "-0", -1, -1, -1,  3);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "*", "*", "-1", -1, -1, -1,  2);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "*", "*", "-5", 2010, 5, 25, 22);
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "1", "*", "*", "-1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "1", "*", "-1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "*", "1", "-1");
    
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "*",  "-0",  "0", -1, -1, 26, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "*",  "-1",  "0", -1, -1, 25, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "*", "-30",  "0", 2010, 4, 26, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "*",  "-1", "-0", -1, -1, 25, 3);
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "*", "-1", "-1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "*", "-1",  "*");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "1", "-1",  "0");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "1", "*", "-1",  "0");
    
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "-0",   "1",  "0", -1,  5,  1, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "-1",   "1",  "0", -1,  4,  1, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "-6",   "1",  "0", 2009, 11,  1, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "*", "-6",  "-0", "-0", 2009, 11, 26, 3);
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "-1",  "1", "-1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "-1", "-1",  "0");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "-1",  "1",  "*");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "*", "-1",  "*",  "0");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "1", "-1",  "1",  "0");
    
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "-0",  "1",  "1",  "0", 2010,  1,  1, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "-1",  "1",  "1",  "0", 2009,  1,  1, 0);
    checkReplaceRelativeValues(tc, 2010, 5, 26, 3, "-1", "-0", "-0", "-0", 2009,  5, 26, 3);
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "-1",  "*",   "1",  "1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "-1",  "1",   "*",  "1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "-1",  "1",   "1",  "*");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "-1", "-1",   "1",  "1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "-1",  "1",  "-1",  "1");
    checkReplaceRelativeValuesInvalid(tc, 2010, 5, 26, 3, "-1",  "1",   "1", "-1");
    
    struct tm t1 = {0, 0, 3, 26, 4, 72, 0, 0, -1};
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("1990",  "*",  "*", "*", "-1"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("*",    "12",  "*", "*", "-1"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("*",     "*", "31", "*", "-1"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("2001",  "*", "-1", "*",  "1"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("*",    "12", "-1", "*",  "1"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("2002", "-1",  "*", "*",  "1"), mktime(&t1))); 
    
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("-1",  "1",  "1", "*", "*"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("-1",  "1",  "*", "*", "0"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("-1",  "*",  "1", "*", "0"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("*",  "-1",  "1", "*", "*"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("*",  "-1",  "*", "*", "0"), mktime(&t1))); 
    CuAssertIntEquals(tc, 0, replaceRelativeValues(makeDateCriteria("*",  "*",  "-1", "*", "*"), mktime(&t1))); 
}
void testFindFirstMatchingDate(CuTest* tc){    
    checkFirstMatchingDate(tc, 2010, 5, 12, 0, "*", "*", "*", "*", "*", 2010, 5, 12, 0); 
    
    checkFirstMatchingDate(tc, 2010,  5, 12,  0,      "2010", "*", "*", "*", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  3,        "-0", "1", "*", "1", "0", 2010,  1,  1,  0); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  0, "2000-2020", "*", "*", "*", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  9, "2000,2010", "*", "*", "*", "*", 2010,  5, 12,  9); 
    checkFirstMatchingDate(tc, 2010, 11,  1,  0, "2010,2011", "*", "*", "*", "*", 2010, 11,  1,  0); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  3,      "2009", "*", "*", "*", "*", 2009, 12, 31, 23); 
    checkFirstMatchingDate(tc, 2020,  1,  1,  6,      "2007", "*", "*", "*", "*", 2007, 12, 31, 23); 
    checkFirstMatchingDate(tc, 2005,  1,  1,  0,      "2005", "*", "*", "*", "*", 2005,  1,  1,  0); 
    checkFirstMatchingDate(tc, 2005, 12, 31,  0,      "2005", "*", "*", "*", "*", 2005, 12, 31,  0); 
    checkFirstMatchingDate(tc, 2005, 12, 31, 23,      "2005", "*", "*", "*", "*", 2005, 12, 31, 23); 
    checkFirstMatchingDate(tc, 2005, 12, 31, 23,        "-7", "1", "*", "1", "0", 1998,  1,  1,  0); 

    checkNoMatchingDates(tc, 2010, 5, 12, 0, "2011",           "*", "*", "*", "*");
    checkNoMatchingDates(tc, 2010, 5, 12, 0, "2011,2015-2020", "*", "*", "*", "*");

    checkFirstMatchingDate(tc, 2010,  5, 12,  0, "*",       "5", "*", "*", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  0, "*",       "4", "*", "*", "*", 2010,  4, 30, 23); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  0, "*",       "9", "*", "*", "*", 2009,  9, 30, 23); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  0, "*", "1,3-4,9", "*", "*", "*", 2010,  4, 30, 23); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  0, "*",      "-2", "*", "1", "0", 2010,  3,  1,  0);       
    checkFirstMatchingDate(tc, 2010,  5, 12,  0, "*",     "-12", "*", "1", "1", 2009,  5,  1,  1); 
    checkFirstMatchingDate(tc, 2010,  5, 12,  1, "*",      "-0", "*", "1", "0", 2010,  5,  1,  0); 
    checkFirstMatchingDate(tc, 2000,  1,  1,  0, "*",      "-0", "*", "1", "0", 2000,  1,  1,  0); 
    checkFirstMatchingDate(tc, 2000, 12, 31, 23, "*",      "-0", "*", "1", "0", 2000, 12,  1,  0); 
                                                                                               
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "2",  "*", "*", 2010,  5, 11, 16); 
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "1",  "*", "*", 2010,  5, 10, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "0",  "*", "*", 2010,  5,  9, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "6",  "*", "*", 2010,  5,  8, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "5",  "*", "*", 2010,  5,  7, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "4",  "*", "*", 2010,  5,  6, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "3",  "*", "*", 2010,  5,  5, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 16, "*", "*", "1,2",  "*", "*", 2010,  5, 11, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 16, "*", "*", "4-6",  "*", "*", 2010,  5,  8, 23); 
    checkFirstMatchingDate(tc, 2010, 5,  1,  0, "*", "*",   "6",  "*", "*", 2010,  5,  1,  0); 
    checkFirstMatchingDate(tc, 2010, 5,  1,  0, "*", "*",   "5",  "*", "*", 2010,  4, 30, 23); 
    checkFirstMatchingDate(tc, 2010, 1,  1,  0, "*", "*",   "5",  "*", "*", 2010,  1,  1,  0);
    checkFirstMatchingDate(tc, 2010, 1,  1,  0, "*", "*",   "4",  "*", "*", 2009, 12, 31, 23);
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "4",  "1", "*", 2010,  4,  1, 23);
    checkFirstMatchingDate(tc, 2010, 5, 11, 16, "*", "*",   "2", "30", "2", 2010,  3, 30,  2);
    
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",     "12", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",     "19", "*", 2010,  4, 19, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",      "7", "*", 2010,  5,  7, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",     "-2", "0", 2010,  5, 10,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",    "-12", "0", 2010,  4, 30,  0); 
    checkFirstMatchingDate(tc, 2020, 1,  7, 14, "*", "*", "*",     "-8", "0", 2019, 12, 30,  0); 
    checkFirstMatchingDate(tc, 2020, 1,  1,  0, "*", "*", "*",     "-0", "0", 2020,  1,  1,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",  "1,2,3", "*", 2010,  5,  3, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",    "1-5", "*", 2010,  5,  5, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",   "1-12", "*", 2010,  5, 12,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*",  "20-25", "*", 2010,  4, 25, 23); 

    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*", "*",    "-0", 2010,  5, 12,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*", "*",    "-1", 2010,  5, 11, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12,  0, "*", "*", "*", "*",    "-1", 2010,  5, 11, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*", "*",    "15", 2010,  5, 12, 15); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*", "*",    "12", 2010,  5, 12, 12); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*", "*", "1,2-7", 2010,  5, 12,  7); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*", "*",    "16", 2010,  5, 11, 16); 
    checkFirstMatchingDate(tc, 2010, 1,  1,  0, "*", "*", "*", "*",    "16", 2009, 12, 31, 16); 

    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "2010", "5", "*",  "*",  "*", 2010, 5, 12, 15); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "2008", "5", "*",  "*",  "*", 2008, 5, 31, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "2008", "5", "*", "31",  "*", 2008, 5, 31, 23); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "2008", "5", "*", "31", "23", 2008, 5, 31, 23); 
    checkNoMatchingDates(tc, 2010, 5, 12,  0, "2010", "6", "*",  "*",     "*");
    checkNoMatchingDates(tc, 2010, 5, 12, 15, "2010", "5", "*", "31",     "*"); 
    checkNoMatchingDates(tc, 2010, 5, 12, 15, "2010", "5", "*", "12", "16-23"); 

    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "-1", "1", "*", "1", "0", 2009, 1,  1,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "-1", "*", "1", "0", 2010,  4,  1,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "-6", "*", "1", "0", 2009, 11,  1,  0); 
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "2009", "11", "*", "*", "0", 2009, 11, 30,  0);

    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "-6", "*", "-0", "-0", 2009, 11, 12, 15);
    
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*",  "-1", "0", 2010, 5, 11,  0);
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*", "-15", "0", 2010, 4, 27,  0);
    
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*",  "*",  "-1", 2010, 5, 12, 14);
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*",  "*", "-20", 2010, 5, 11, 19);
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*",  "*", "-24", 2010, 5, 11, 15);
    
 // Common scenarios
 
 // First day of month
    checkFirstMatchingDate(tc, 2010, 5,  1,  0, "*", "*", "*", "1", "0", 2010, 5, 1, 0);
    checkFirstMatchingDate(tc, 2010, 5, 12, 15, "*", "*", "*", "1", "0", 2010, 5, 1, 0);
    checkFirstMatchingDate(tc, 2010, 5, 31, 23, "*", "*", "*", "1", "0", 2010, 5, 1, 0);
    checkFirstMatchingDate(tc, 2010, 6,  1,  0, "*", "*", "*", "1", "0", 2010, 6, 1, 0);
    
 // Rolling 30 day period
    checkFirstMatchingDate(tc, 2010, 5,  1,  0, "*", "*", "*", "-30", "-0", 2010, 4,  1,  0);
    checkFirstMatchingDate(tc, 2010, 5, 10,  3, "*", "*", "*", "-30", "-0", 2010, 4, 10,  3);
    checkFirstMatchingDate(tc, 2010, 5, 31, 23, "*", "*", "*", "-30", "-0", 2010, 5,  1, 23);
    
 // Rolling month
    //checkFirstMatchingDate(tc, 2010, 7, 31,  0, "*", "-1", "*",  "-0", "-0", 2010, 6,  30,  0); TODO broken, returns 1/6/10

 // Since Sunday
    checkFirstMatchingDate(tc, 2010, 5, 11,  0, "*", "*", "0", "*", "*", 2010, 5, 9, 23);
    checkFirstMatchingDate(tc, 2010, 5, 10,  0, "*", "*", "0", "*", "*", 2010, 5, 9, 23);
    checkFirstMatchingDate(tc, 2010, 5,  9,  2, "*", "*", "0", "*", "*", 2010, 5, 9,  2);
    checkFirstMatchingDate(tc, 2010, 5,  8, 23, "*", "*", "0", "*", "*", 2010, 5, 2, 23);
}
void testGetTotalsForAlert(CuTest* tc){
    emptyDb();
    setTime(makeTs("2010-05-30 12:00:00"));

    struct Alert* alert = allocAlert();
    alert->id        = 1;
    alert->active    = 1;
    alert->direction = 1;
    alert->amount    = 10000;
    alert->bound     = makeDateCriteria("2010", "5", "1", "*", "0");
    setAlertName(alert, "alert1");
    checkAlertTotals(tc, alert, 0, 0);
    
    addDbRow(makeTs("2010-04-30 23:00:00"), 1, "eth0", 1, 1, NULL);
    checkAlertTotals(tc, alert, 0, 0);

    addDbRow(makeTs("2010-05-01 00:00:00"), 1, "eth0", 2, 2, NULL);
    checkAlertTotals(tc, alert, 0, 0);

    addDbRow(makeTs("2010-05-01 01:00:00"), 1, "eth0", 4, 4, NULL);
    checkAlertTotals(tc, alert, 4, 0);

    addDbRow(makeTs("2010-05-10 22:00:00"), 1, "eth1", 8, 8, NULL);
    checkAlertTotals(tc, alert, 12, 0);

    addDbRow(makeTs("2010-05-10 23:00:00"), 1, "eth1", 16, 16, NULL);
    checkAlertTotals(tc, alert, 28, 0);
    
    alert->direction = 2;
    checkAlertTotals(tc, alert, 0, 28);
    alert->direction = 0;
    checkAlertTotals(tc, alert, 0, 0);
    alert->direction = 3;
    checkAlertTotals(tc, alert, 28, 28);
    
    emptyDb();
    
    addDbRow(makeTs("2010-05-01 12:00:00"), 3600, "eth0", 1, 1, NULL); // Saturday (no)
    addDbRow(makeTs("2010-05-01 13:00:00"), 3600, "eth0", 2, 2, NULL); // Saturday (yes)
    addDbRow(makeTs("2010-05-01 16:00:00"), 3600, "eth0", 4, 4, NULL); // Saturday (yes)
    addDbRow(makeTs("2010-05-01 17:00:00"), 3600, "eth0", 8, 8, NULL); // Saturday (yes) '12-16' = 12:**-16:**
    addDbRow(makeTs("2010-05-01 18:00:00"), 3600, "eth0", 1, 1, NULL); // Saturday (no)
    
    addDbRow(makeTs("2010-05-02 12:00:00"), 3600, "eth0",  1,  1, NULL); // Sunday (no)
    addDbRow(makeTs("2010-05-02 13:00:00"), 3600, "eth0", 16, 16, NULL); // Sunday (yes)
    addDbRow(makeTs("2010-05-02 16:00:00"), 3600, "eth0", 32, 32, NULL); // Sunday (yes)
    addDbRow(makeTs("2010-05-02 17:00:00"), 3600, "eth0", 64, 64, NULL); // Sunday (yes) '12-16' = 12:**-16:**
    addDbRow(makeTs("2010-05-02 18:00:00"), 3600, "eth0",  1,  1, NULL); // Sunday (no)

    addDbRow(makeTs("2010-05-03 06:00:00"), 3600, "eth0",    1,    1, NULL); // Monday (no)
    addDbRow(makeTs("2010-05-03 07:00:00"), 3600, "eth0",  128,  128, NULL); // Monday (yes)
    addDbRow(makeTs("2010-05-03 12:00:00"), 3600, "eth0",  256,  256, NULL); // Monday (yes)
    addDbRow(makeTs("2010-05-03 19:00:00"), 3600, "eth0", 1024, 1024, NULL); // Monday (yes) '6-18' = 06:**-18:**
    addDbRow(makeTs("2010-05-03 20:00:00"), 3600, "eth0",    1,    1, NULL); // Monday (no)

    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "1-5", "6-18");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "0,6", "12-16");
    period3->next = period4;
    alert->periods = period3;
    
    checkAlertTotals(tc, alert, 1534, 1534);     
    alert->direction = 0;
    checkAlertTotals(tc, alert, 0, 0);
    alert->direction = 1;
    checkAlertTotals(tc, alert, 1534, 0);
    alert->direction = 2;
    checkAlertTotals(tc, alert, 0, 1534);
}

int callback(void* a, int b, char** c, char** d){
    count++;
}

CuSuite* clientAlertSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testAddGetRemoveAlerts);
    SUITE_ADD_TEST(suite, testRemoveAlertsDb);
    SUITE_ADD_TEST(suite, testIsDateCriteriaPartMatch);
    SUITE_ADD_TEST(suite, testIsDateCriteriaMatch);
    SUITE_ADD_TEST(suite, testFindLowestMatch);
    SUITE_ADD_TEST(suite, testFindHighestMatchAtOrBelowLimit);
    SUITE_ADD_TEST(suite, testFindHighestMatch);
    SUITE_ADD_TEST(suite, testGetNonRelativeValue);
    SUITE_ADD_TEST(suite, testReplaceRelativeValues);
    SUITE_ADD_TEST(suite, testFindFirstMatchingDate);
    SUITE_ADD_TEST(suite, testGetTotalsForAlert);
    return suite;
}
