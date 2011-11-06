#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"

static void checkPartToText(char* txt){
    struct DateCriteriaPart* part = makeDateCriteriaPart(txt);
    char* result = dateCriteriaPartToText(part);
    assert_string_equal(txt, result);
    free(result);
    freeDateCriteriaPart(part);
}

void testAllocAlert(void **state){
    struct Alert* alert = allocAlert();
    assert_true(NULL == alert->name);
    assert_true(NULL == alert->bound);
    assert_true(NULL == alert->periods);
    assert_true(NULL == alert->next);
    assert_int_equal(0, alert->id);
    assert_int_equal(0, alert->active);
    assert_int_equal(0, alert->filter);
    assert_int_equal(0, alert->amount);
    
    freeAlert(alert);
}

void testSetAlertName(void **state){     
    struct Alert* alert = allocAlert();   
    setAlertName(alert, "test");
    assert_string_equal("test", alert->name);
    setAlertName(alert, "  test2 ");
    assert_string_equal("test2", alert->name);
    setAlertName(alert, NULL);
    assert_true(NULL == alert->name);
    
    freeAlert(alert);
}

void testAppendAlert(void **state){    
    struct Alert* alertBase = NULL;
    struct Alert* alert1 = allocAlert();
    alert1->id = 1;
    appendAlert(&alertBase, alert1);
    assert_int_equal(1, alertBase->id);
    
    struct Alert* alert2 = allocAlert();
    alert2->id = 2;
    appendAlert(&alertBase, alert2);
    assert_int_equal(1, alertBase->id);
    assert_int_equal(2, alertBase->next->id);
    
    struct Alert* alert3 = allocAlert();
    alert3->id = 3;
    appendAlert(&alert2, alert3);
    assert_int_equal(1, alertBase->id);
    assert_int_equal(2, alertBase->next->id);
    assert_int_equal(3, alertBase->next->next->id);
    
    freeAlert(alert1);
}
    
    
static void checkDateCriteriaPartIsNull(char* txt){
    struct DateCriteriaPart* part = makeDateCriteriaPart(txt);
    assert_true(NULL == part);
}    

void checkDateCriteriaPartNotNull(char* txt, int isRelative, int val1, int val2, int next){
    struct DateCriteriaPart* part = makeDateCriteriaPart(txt);
    checkDateCriteriaPart(part, isRelative, val1, val2, next);
    freeDateCriteriaPart(part);
}    

void testMakeDateCriteriaPart(void **state){    
    checkDateCriteriaPartIsNull("*");
    checkDateCriteriaPartIsNull("x");

    checkDateCriteriaPartNotNull("12", 0, 12, 12, 0);
    checkDateCriteriaPartNotNull("-12", 1, 12, 0, 0);
    checkDateCriteriaPartIsNull("-");
    checkDateCriteriaPartIsNull("-x");
    checkDateCriteriaPartIsNull("1-");
    checkDateCriteriaPartIsNull("x-");
    checkDateCriteriaPartNotNull("11-12", 0, 11, 12, 0);
    checkDateCriteriaPartNotNull("12-12", 0, 12, 12, 0);
    checkDateCriteriaPartIsNull("12-11");
    checkDateCriteriaPartIsNull("x-12");
    checkDateCriteriaPartIsNull("11-x");
    
    checkDateCriteriaPartIsNull("1,2,x");
    checkDateCriteriaPartIsNull("1,2-x,3");
    
    struct DateCriteriaPart* firstPart = makeDateCriteriaPart("1,2-3,3");
    struct DateCriteriaPart* part = firstPart;
    checkDateCriteriaPart(part, 0, 1, 1, 1);
    part = part->next;
    checkDateCriteriaPart(part, 0, 2, 3, 1);
    part = part->next;
    checkDateCriteriaPart(part, 0, 3, 3, 0);
    freeDateCriteriaPart(firstPart);
    
    firstPart = part = makeDateCriteriaPart("1,2,");
    checkDateCriteriaPart(part, 0, 1, 1, 1);
    part = part->next;
    checkDateCriteriaPart(part, 0, 2, 2, 0);
    freeDateCriteriaPart(firstPart);
    
    firstPart = part = makeDateCriteriaPart(",1,2");
    checkDateCriteriaPart(part, 0, 1, 1, 1);
    part = part->next;
    checkDateCriteriaPart(part, 0, 2, 2, 0);
    freeDateCriteriaPart(firstPart);
}

void testDateCriteriaPartToText(void **state){    
    checkPartToText("*");
    checkPartToText("1");
    checkPartToText("1,4,8");
    checkPartToText("-9");
    checkPartToText("6-9,2,10-100");
}

void testMakeDateCriteria(void **state){    
    struct DateCriteria* criteria = makeDateCriteria("*", "0,4-5,11", "1-20", "*", "5");
    assert_true(NULL == criteria->year);
    
    struct DateCriteriaPart* monthPart = criteria->month;
    checkDateCriteriaPart(monthPart, 0, 0, 0, 1);
    monthPart = monthPart->next;
    checkDateCriteriaPart(monthPart, 0, 4, 5, 1);
    monthPart = monthPart->next;
    checkDateCriteriaPart(monthPart, 0, 11, 11, 0);
    
    checkDateCriteriaPart(criteria->day, 0, 1, 20, 0);
    
    assert_true(NULL == criteria->weekday);
    
    checkDateCriteriaPart(criteria->hour, 0, 5, 5, 0);
    freeDateCriteria(criteria);
}

void testAppendDateCriteria(void **state){
    struct DateCriteria* baseCriteria = NULL;
    struct DateCriteria* criteria1 = makeDateCriteria("*", "*", "*", "*", "1");
 
    appendDateCriteria(&baseCriteria, criteria1);
    assert_true(baseCriteria == criteria1);
 
    struct DateCriteria* criteria2 = makeDateCriteria("*", "*", "*", "*", "2");
    appendDateCriteria(&baseCriteria, criteria2);
    assert_true(baseCriteria == criteria1);
    assert_true(baseCriteria->next == criteria2);
 
    struct DateCriteria* criteria3 = makeDateCriteria("*", "*", "*", "*", "3");
    appendDateCriteria(&criteria2, criteria3);
    assert_true(baseCriteria == criteria1);
    assert_true(baseCriteria->next == criteria2);
    assert_true(baseCriteria->next->next == criteria3);
    
    freeDateCriteria(baseCriteria);
}    
