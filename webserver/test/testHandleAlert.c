#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "bmws.h"

/*
Contains unit tests for the handleAlert module.
*/

void testAlertNoAction(void** state) {
 // The 'action' parameter is required, so we should get an HTTP error if its missing
    struct Request req = {"GET", "/alert", NULL, NULL};

	expect_string(mockWriteHeadersServerError, msg, "Missing/invalid 'action' parameter: '%s'");    
	
    processAlertRequest(0, &req, FALSE);
}

void testAlertListNone(void** state) {
 	struct NameValuePair param = {"action", "list", NULL};
    struct Request req = {"GET", "/alert", &param, NULL};
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);
    expect_string(mockWriteText, txt, "[");
    expect_string(mockWriteText, txt, "]");
    
    processAlertRequest(0, &req, FALSE);
    
    freeStmtList();
}

void testAlertList(void** state) {
	struct Alert* alert1 = allocAlert();
    alert1->active = 1;
    alert1->filter = 1;
    alert1->amount = 100000000000;
    alert1->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);
    freeAlert(alert1);
    
	struct Alert* alert2 = allocAlert();
    alert2->active = 0;
    alert2->filter = 2;
    alert2->amount = 200000000000;
    alert2->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");
    
    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);
    freeAlert(alert2);
    
 	struct NameValuePair param = {"action", "list", NULL};
    struct Request req = {"GET", "/alert", &param, NULL};
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);

    expect_string(mockWriteText, txt, "[");
    
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteNumValueToJson, key, "id");
    expect_value(mockWriteNumValueToJson, value, 1);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteTextValueToJson, key, "name");
    expect_string(mockWriteTextValueToJson, value, "alert1");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "active");
    expect_value(mockWriteNumValueToJson, value, 1);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "filter");
    expect_value(mockWriteNumValueToJson, value, 1);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "amount");
    expect_value(mockWriteNumValueToJson, value, 100000000000);
    expect_string(mockWriteText, txt, ",");
    
    expect_string(mockWriteTextArrayToJson, key, "bound");
    expect_string(mockWriteTextArrayToJsonValue, value, "2010");
    expect_string(mockWriteTextArrayToJsonValue, value, "5");
    expect_string(mockWriteTextArrayToJsonValue, value, "26");
    expect_string(mockWriteTextArrayToJsonValue, value, "4");
    expect_string(mockWriteTextArrayToJsonValue, value, "15");
    
    expect_string(mockWriteText, txt, ",\"periods\" : [");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "5,6");
    expect_string(mockWriteTextArrayToJsonValue, value, "0-6");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "0-4");
    expect_string(mockWriteTextArrayToJsonValue, value, "6-12");
    expect_string(mockWriteText, txt, "]}");
    
    expect_string(mockWriteText, txt, ",");

	expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteNumValueToJson, key, "id");
    expect_value(mockWriteNumValueToJson, value, 2);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteTextValueToJson, key, "name");
    expect_string(mockWriteTextValueToJson, value, "alert2");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "active");
    expect_value(mockWriteNumValueToJson, value, 0);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "filter");
    expect_value(mockWriteNumValueToJson, value, 2);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "amount");
    expect_value(mockWriteNumValueToJson, value, 200000000000);
    expect_string(mockWriteText, txt, ",");
    
    expect_string(mockWriteTextArrayToJson, key, "bound");
    expect_string(mockWriteTextArrayToJsonValue, value, "2010");
    expect_string(mockWriteTextArrayToJsonValue, value, "5");
    expect_string(mockWriteTextArrayToJsonValue, value, "26");
    expect_string(mockWriteTextArrayToJsonValue, value, "4");
    expect_string(mockWriteTextArrayToJsonValue, value, "15");
    
    expect_string(mockWriteText, txt, ",\"periods\" : [");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "2,8");
    expect_string(mockWriteTextArrayToJsonValue, value, "5");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteTextArrayToJsonValue, value, "1");
    expect_string(mockWriteTextArrayToJsonValue, value, "*");
    expect_string(mockWriteText, txt, "]}");
    
    expect_string(mockWriteText, txt, "]");
    
    processAlertRequest(0, &req, FALSE);
    
    freeStmtList();
}

void testAlertDeleteOk(void** state) {
	struct Alert* alert1 = allocAlert();
    alert1->active = 1;
    alert1->filter = 1;
    alert1->amount = 100000000000;
    alert1->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);
    freeAlert(alert1);
    
	struct Alert* alert2 = allocAlert();
    alert2->active = 0;
    alert2->filter = 2;
    alert2->amount = 200000000000;
    alert2->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");
    
    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);
    freeAlert(alert2);
    
    struct Alert* alert = getAlerts();
    assert_int_equal(alert->id, 1);
    assert_int_equal(alert->next->id, 2);
    assert_true(alert->next->next == NULL);
    freeAlert(alert);
    
 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"action", "delete", &param1};
    struct Request req = {"GET", "/alert", &param2, NULL};
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);

    expect_string(mockWriteText, txt, "{}");

    processAlertRequest(0, &req, TRUE);
    
    alert = getAlerts();
    assert_int_equal(alert->id, 2);
    assert_true(alert->next == NULL);
    freeAlert(alert);
    
    freeStmtList();
}

void testAlertDeleteForbidden(void** state) {
	struct Alert* alert1 = allocAlert();
    alert1->active = 1;
    alert1->filter = 1;
    alert1->amount = 100000000000;
    alert1->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);
    freeAlert(alert1);
    
	struct Alert* alert2 = allocAlert();
    alert2->active = 0;
    alert2->filter = 2;
    alert2->amount = 200000000000;
    alert2->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");
    
    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);
    freeAlert(alert2);
    
    struct Alert* alert = getAlerts();
    assert_int_equal(alert->id, 1);
    assert_int_equal(alert->next->id, 2);
    assert_true(alert->next->next == NULL);
    freeAlert(alert);
    
 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"action", "delete", &param1};
    struct Request req = {"GET", "/alert", &param2, NULL};
                                                    
	expect_string(mockWriteHeadersForbidden, msg, "alert delete");
    processAlertRequest(0, &req, FALSE);
    
    alert = getAlerts();
    assert_int_equal(alert->id, 1);
    assert_int_equal(alert->next->id, 2);
    assert_true(alert->next->next == NULL);
    freeAlert(alert);
    
    freeStmtList();
}

static void checkAlertUpdateMissingArg(struct NameValuePair param){
	struct NameValuePair paramAction = {"action", "update", &param};
	struct Request req = {"GET", "/alert", &paramAction, NULL};
    expect_string(mockWriteHeadersServerError, msg, "processAlertUpdate param bad/missing id=%s, name=%s, active=%s, filter=%s, amount=%s, bound=%s, periods=%s");
    processAlertRequest(0, &req, TRUE);
}

void testAlertUpdateMissingArgs(void** state) {
 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"name", "alert1", &param1};
 	struct NameValuePair param3 = {"active", "1", &param2};
 	struct NameValuePair param4 = {"filter", "1", &param3};
 	struct NameValuePair param5 = {"amount", "100", &param4};
 	struct NameValuePair param6 = {"bound", "2010,5,26,4,15", &param5};
 	struct NameValuePair param7 = {"periods", "2010,5,26,4,15", &param6};
    
 // Missing 'id' parameter
  	param2.next = NULL;
    checkAlertUpdateMissingArg(param7);
	param2.next = &param1;
    
 // Missing 'name' param
 	param3.next = &param1;
 	checkAlertUpdateMissingArg(param7);
 	param3.next = &param2;
    
 // Missing 'active' param
 	param4.next = &param2;
 	checkAlertUpdateMissingArg(param7);
 	param4.next = &param3;
    
 // Missing 'direction' param
 	param5.next = &param3;
 	checkAlertUpdateMissingArg(param7);
 	param5.next = &param4;
    
 // Missing 'amount' param
 	param6.next = &param4;
 	checkAlertUpdateMissingArg(param7);
 	param6.next = &param5;
    
 // Missing 'bound' param
 	param7.next = &param5;
 	checkAlertUpdateMissingArg(param7);
 	param7.next = &param6;
    
 // Missing 'periods' param
 	checkAlertUpdateMissingArg(param6);
}

void testAlertUpdateOk(void** state) {
	struct Alert* alert1 = allocAlert();
    alert1->active = 1;
    alert1->filter = 1;
    alert1->amount = 100000000000;
    alert1->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);
	freeAlert(alert1);
    
	struct Alert* alert2 = allocAlert();
    alert2->active = 0;
    alert2->filter = 2;
    alert2->amount = 200000000000;
    alert2->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");
    
    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);
    freeAlert(alert2);
    
    struct Alert* alert = getAlerts();
    assert_int_equal(alert->id, 1);
    assert_int_equal(alert->next->id, 2);
    assert_true(alert->next->next == NULL);
    freeAlert(alert);
    
 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"name", "newname", &param1};
 	struct NameValuePair param3 = {"active", "0", &param2};
 	struct NameValuePair param4 = {"filter", "2", &param3};
 	struct NameValuePair param5 = {"amount", "200", &param4};
 	struct NameValuePair param6 = {"bound", "['2009','4','25','3','14']", &param5};
 	struct NameValuePair param7 = {"periods", "[['2011','6','27','5','16']]", &param6};
 	struct NameValuePair param8 = {"action", "update", &param7};
    
    struct Request req = {"GET", "/alert", &param8, NULL};
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);

    expect_string(mockWriteText, txt, "{}");

    processAlertRequest(0, &req, TRUE);
    
    alert = getAlerts();
    assert_int_equal(2, alert->id);
    
    struct Alert* firstAlert = alert;
    
	alert = alert->next;
    assert_int_equal(1, alert->id);
    assert_string_equal("newname", alert->name);
    assert_int_equal(0, alert->active);
    assert_int_equal(2, alert->filter);
    assert_int_equal(200, alert->amount);
    
    struct DateCriteria* bound = alert->bound;
    checkDateCriteriaPart(bound->year,    FALSE, 2009, 2009, FALSE);
	checkDateCriteriaPart(bound->month,   FALSE, 4, 4, FALSE);
	checkDateCriteriaPart(bound->day,     FALSE, 25, 25, FALSE);
	checkDateCriteriaPart(bound->weekday, FALSE, 3, 3, FALSE);
	checkDateCriteriaPart(bound->hour,    FALSE, 14, 14, FALSE);
    
	struct DateCriteria* period = alert->periods;
    checkDateCriteriaPart(period->year,    FALSE, 2011, 2011, FALSE);
	checkDateCriteriaPart(period->month,   FALSE, 6, 6, FALSE);
	checkDateCriteriaPart(period->day,     FALSE, 27, 27, FALSE);
	checkDateCriteriaPart(period->weekday, FALSE, 5, 5, FALSE);
	checkDateCriteriaPart(period->hour,    FALSE, 16, 16, FALSE);
    
	assert_true(period->next == NULL);
    freeAlert(firstAlert);
	
	freeStmtList();
}

void testAlertUpdateForbidden(void** state) {
	struct Alert* alert1 = allocAlert();
    alert1->active = 1;
    alert1->filter = 1;
    alert1->amount = 100000000000;
    alert1->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");
    
    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);
    freeAlert(alert1);
    
	struct Alert* alert2 = allocAlert();
    alert2->active = 0;
    alert2->filter = 2;
    alert2->amount = 200000000000;
    alert2->bound  = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");
    
    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);
    freeAlert(alert2);
    
    struct Alert* alert = getAlerts();
    assert_int_equal(alert->id, 1);
    assert_int_equal(alert->next->id, 2);
    assert_true(alert->next->next == NULL);
    freeAlert(alert);
    
 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"name", "newname", &param1};
 	struct NameValuePair param3 = {"active", "0", &param2};
 	struct NameValuePair param4 = {"filter", "2", &param3};
 	struct NameValuePair param5 = {"amount", "200", &param4};
 	struct NameValuePair param6 = {"bound", "['2009','4','25','3','14']", &param5};
 	struct NameValuePair param7 = {"periods", "[['2011','6','27','5','16']]", &param6};
 	struct NameValuePair param8 = {"action", "update", &param7};
    
    struct Request req = {"GET", "/alert", &param8, NULL};
    
    expect_string(mockWriteHeadersForbidden, msg, "alert update");
    processAlertRequest(0, &req, FALSE);
    
    alert = getAlerts();
    assert_int_equal(alert->id, 1);
    assert_int_equal(alert->next->id, 2);
    assert_true(alert->next->next == NULL);    
    freeAlert(alert);
    
    freeStmtList();
}

void testProcessAlertStatus(void** state) {
	struct Alert* alert1 = allocAlert();
    alert1->active = 1;
    alert1->filter = 1;
    alert1->amount = 1000000;
    alert1->bound  = makeDateCriteria("2009", "1", "1", "4", "1");
    setAlertName(alert1, "alert1");
    
    struct DateCriteria* period = makeDateCriteria("*", "*", "*", "*", "*");
    alert1->periods = period;
    
	addAlert(alert1);
    freeAlert(alert1);
    
 	struct NameValuePair param = {"action", "status", NULL};
    
    struct Request req = {"GET", "/alert", &param, NULL};
	addDbRow(makeTs("2009-11-01 00:00:00"), 3600, 200000, 1);
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);

    expect_string(mockWriteText, txt, "[");
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteNumValueToJson, key, "id");
    expect_value(mockWriteNumValueToJson, value, 1);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteTextValueToJson, key, "name");
    expect_string(mockWriteTextValueToJson, value, "alert1");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "current");
    expect_value(mockWriteNumValueToJson, value, 200000);
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteNumValueToJson, key, "limit");
    expect_value(mockWriteNumValueToJson, value, 1000000);
    expect_string(mockWriteText, txt, "}");

    expect_string(mockWriteText, txt, "]");

    processAlertStatus(0, &req, FALSE);
    
    freeStmtList();
}

