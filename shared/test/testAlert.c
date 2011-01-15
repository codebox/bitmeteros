/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "CuTest.h"
#include "test.h"

static void checkPartToText(CuTest *tc, char* txt){
    CuAssertStrEquals(tc,txt, dateCriteriaPartToText(makeDateCriteriaPart(txt)));
}

void testAllocAlert(CuTest *tc){
    struct Alert* alert = allocAlert();
    CuAssertTrue(tc, NULL == alert->name);
    CuAssertTrue(tc, NULL == alert->bound);
    CuAssertTrue(tc, NULL == alert->periods);
    CuAssertTrue(tc, NULL == alert->next);
    CuAssertIntEquals(tc,0, alert->id);
    CuAssertIntEquals(tc,0, alert->active);
    CuAssertIntEquals(tc,0, alert->direction);
    CuAssertIntEquals(tc,0, alert->amount);
}

void testSetAlertName(CuTest *tc){     
	struct Alert* alert = allocAlert();   
    setAlertName(alert, "test");
    CuAssertStrEquals(tc,"test", alert->name);
    setAlertName(alert, "  test2 ");
    CuAssertStrEquals(tc,"test2", alert->name);
    setAlertName(alert, NULL);
    CuAssertTrue(tc, NULL == alert->name);
}

void testAppendAlert(CuTest *tc){    
    struct Alert* alertBase = NULL;
    struct Alert* alert1 = allocAlert();
    alert1->id = 1;
    appendAlert(&alertBase, alert1);
    CuAssertIntEquals(tc,1, alertBase->id);
    
    struct Alert* alert2 = allocAlert();
    alert2->id = 2;
    appendAlert(&alertBase, alert2);
    CuAssertIntEquals(tc,1, alertBase->id);
    CuAssertIntEquals(tc,2, alertBase->next->id);
    
    struct Alert* alert3 = allocAlert();
    alert3->id = 3;
    appendAlert(&alert2, alert3);
    CuAssertIntEquals(tc,1, alertBase->id);
    CuAssertIntEquals(tc,2, alertBase->next->id);
    CuAssertIntEquals(tc,3, alertBase->next->next->id);
}
    
void testMakeDateCriteriaPart(CuTest *tc){    
    struct DateCriteriaPart* part;
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("*"));
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("x"));
    checkDateCriteriaPart(tc,makeDateCriteriaPart("12"), 0, 12, 12, 0);
    checkDateCriteriaPart(tc,makeDateCriteriaPart("-12"), 1, 12, 0, 0);
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("-"));
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("-x"));
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("1-"));
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("x-"));
    checkDateCriteriaPart(tc,makeDateCriteriaPart("11-12"), 0, 11, 12, 0);
    checkDateCriteriaPart(tc,makeDateCriteriaPart("12-12"), 0, 12, 12, 0);
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("12-11"));
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("x-12"));
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("11-x"));
    
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("1,2,x"));
    CuAssertTrue(tc, NULL == makeDateCriteriaPart("1,2-x,3"));
    
    part = makeDateCriteriaPart("1,2-3,3");
    checkDateCriteriaPart(tc,part, 0, 1, 1, 1);
    part = part->next;
    checkDateCriteriaPart(tc,part, 0, 2, 3, 1);
    part = part->next;
    checkDateCriteriaPart(tc,part, 0, 3, 3, 0);
    
    part = makeDateCriteriaPart("1,2,");
    checkDateCriteriaPart(tc,part, 0, 1, 1, 1);
    part = part->next;
    checkDateCriteriaPart(tc,part, 0, 2, 2, 0);

    part = makeDateCriteriaPart(",1,2");
    checkDateCriteriaPart(tc,part, 0, 1, 1, 1);
    part = part->next;
    checkDateCriteriaPart(tc,part, 0, 2, 2, 0);
}

void testDateCriteriaPartToText(CuTest *tc){    
    checkPartToText(tc,"*");
    checkPartToText(tc,"1");
    checkPartToText(tc,"1,4,8");
    checkPartToText(tc,"-9");
    checkPartToText(tc,"6-9,2,10-100");
}

void testMakeDateCriteria(CuTest *tc){    
    struct DateCriteria* criteria = makeDateCriteria("*", "0,4-5,11", "1-20", "*", "5");
    CuAssertTrue(tc, NULL == criteria->year);
    
    struct DateCriteriaPart* monthPart = criteria->month;
    checkDateCriteriaPart(tc,monthPart, 0, 0, 0, 1);
    monthPart = monthPart->next;
    checkDateCriteriaPart(tc,monthPart, 0, 4, 5, 1);
    monthPart = monthPart->next;
    checkDateCriteriaPart(tc,monthPart, 0, 11, 11, 0);
    
    checkDateCriteriaPart(tc,criteria->day, 0, 1, 20, 0);
    
    CuAssertTrue(tc, NULL == criteria->weekday);
    
    checkDateCriteriaPart(tc,criteria->hour, 0, 5, 5, 0);
}

void testAppendDateCriteria(CuTest *tc){
    struct DateCriteria* baseCriteria = NULL;
    struct DateCriteria* criteria1 = makeDateCriteria("*", "*", "*", "*", "1");
 
    appendDateCriteria(&baseCriteria, criteria1);
    CuAssertTrue(tc,baseCriteria == criteria1);
 
    struct DateCriteria* criteria2 = makeDateCriteria("*", "*", "*", "*", "2");
    appendDateCriteria(&baseCriteria, criteria2);
    CuAssertTrue(tc,baseCriteria == criteria1);
    CuAssertTrue(tc,baseCriteria->next == criteria2);
 
    struct DateCriteria* criteria3 = makeDateCriteria("*", "*", "*", "*", "3");
    appendDateCriteria(&criteria2, criteria3);
    CuAssertTrue(tc,baseCriteria == criteria1);
    CuAssertTrue(tc,baseCriteria->next == criteria2);
    CuAssertTrue(tc,baseCriteria->next->next == criteria3);
}    

CuSuite* alertGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testAllocAlert);
    SUITE_ADD_TEST(suite, testSetAlertName);
    SUITE_ADD_TEST(suite, testAppendAlert);
    SUITE_ADD_TEST(suite, testMakeDateCriteriaPart);
    SUITE_ADD_TEST(suite, testDateCriteriaPartToText);
    SUITE_ADD_TEST(suite, testMakeDateCriteria);
    SUITE_ADD_TEST(suite, testAppendDateCriteria);
    return suite;
}
