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
#include "common.h"
#include "test.h"
#include "client.h"
#include "CuTest.h"
#include "bmws.h"

/*
Contains unit tests for the handleAlert module.
*/
void testAlertNoAction(CuTest *tc) {
 // The 'action' parameter is required, so we should get an HTTP error if its missing
    struct Request req = {"GET", "/alert", NULL, NULL};

    int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, FALSE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
		"HTTP/1.0 500 Bad/missing parameter" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
    , result);
}

void testAlertListNone(CuTest *tc) {
 	struct NameValuePair param = {"action", "list", NULL};
    struct Request req = {"GET", "/alert", &param, NULL};

    int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, FALSE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
		"HTTP/1.0 200 OK" HTTP_EOL
		"Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "[]"
    , result);
}

void testAlertList(CuTest *tc) {
	emptyDb();
	struct Alert* alert1 = allocAlert();
    alert1->active    = 1;
    alert1->direction = 1;
    alert1->amount    = 100000000000;
    alert1->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");

    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);

	struct Alert* alert2 = allocAlert();
    alert2->active    = 0;
    alert2->direction = 2;
    alert2->amount    = 200000000000;
    alert2->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");

    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);

 	struct NameValuePair param = {"action", "list", NULL};
    struct Request req = {"GET", "/alert", &param, NULL};

    int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, FALSE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
		"HTTP/1.0 200 OK" HTTP_EOL
		"Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "[{\"id\" : 1,\"name\" : \"alert1\",\"active\" : 1,\"direction\" : 1,\"amount\" : 100000000000,\"bound\" : [\"2010\",\"5\",\"26\",\"4\",\"15\"],\"periods\" : [[\"*\",\"*\",\"*\",\"5,6\",\"0-6\"],[\"*\",\"*\",\"*\",\"0-4\",\"6-12\"]]},"
       	 "{\"id\" : 2,\"name\" : \"alert2\",\"active\" : 0,\"direction\" : 2,\"amount\" : 200000000000,\"bound\" : [\"2010\",\"5\",\"26\",\"4\",\"15\"],\"periods\" : [[\"*\",\"*\",\"*\",\"2,8\",\"5\"],[\"*\",\"*\",\"*\",\"1\",\"*\"]]}]"
    , result);
}

void testAlertDeleteOk(CuTest *tc) {
	emptyDb();
	struct Alert* alert1 = allocAlert();
    alert1->active    = 1;
    alert1->direction = 1;
    alert1->amount    = 100000000000;
    alert1->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");

    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);

	struct Alert* alert2 = allocAlert();
    alert2->active    = 0;
    alert2->direction = 2;
    alert2->amount    = 200000000000;
    alert2->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");

    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);

    struct Alert* alert = getAlerts();
    CuAssertIntEquals(tc, alert->id, 1);
    CuAssertIntEquals(tc, alert->next->id, 2);
    CuAssertTrue(tc, alert->next->next == NULL);

 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"action", "delete", &param1};
    struct Request req = {"GET", "/alert", &param2, NULL};

    int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, TRUE);

	char* result = readTmpFile();
    CuAssertStrEquals(tc,
		"HTTP/1.0 200 OK" HTTP_EOL
		"Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "{}"
    , result);

    alert = getAlerts();
    CuAssertIntEquals(tc, alert->id, 2);
    CuAssertTrue(tc, alert->next == NULL);
}

void testAlertDeleteForbidden(CuTest *tc) {
	emptyDb();
	struct Alert* alert1 = allocAlert();
    alert1->active    = 1;
    alert1->direction = 1;
    alert1->amount    = 100000000000;
    alert1->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");

    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);

	struct Alert* alert2 = allocAlert();
    alert2->active    = 0;
    alert2->direction = 2;
    alert2->amount    = 200000000000;
    alert2->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");

    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);

    struct Alert* alert = getAlerts();
    CuAssertIntEquals(tc, alert->id, 1);
    CuAssertIntEquals(tc, alert->next->id, 2);
    CuAssertTrue(tc, alert->next->next == NULL);

 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"action", "delete", &param1};
    struct Request req = {"GET", "/alert", &param2, NULL};

    int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, FALSE);

	char* result = readTmpFile();
    CuAssertStrEquals(tc,
		"HTTP/1.0 403 Forbidden" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
    , result);

    alert = getAlerts();
    CuAssertIntEquals(tc, alert->id, 1);
    CuAssertIntEquals(tc, alert->next->id, 2);
    CuAssertTrue(tc, alert->next->next == NULL);

}

static void checkAlertUpdateMissingArg(CuTest *tc, struct NameValuePair param){
	struct NameValuePair paramAction = {"action", "update", &param};
	struct Request req = {"GET", "/alert", &paramAction, NULL};

	int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, TRUE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
		"HTTP/1.0 500 Bad/missing parameter" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
    , result);
}

void testAlertUpdateMissingArgs(CuTest *tc) {
 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"name", "alert1", &param1};
 	struct NameValuePair param3 = {"active", "1", &param2};
 	struct NameValuePair param4 = {"direction", "1", &param3};
 	struct NameValuePair param5 = {"amount", "100", &param4};
 	struct NameValuePair param6 = {"bound", "2010,5,26,4,15", &param5};
 	struct NameValuePair param7 = {"periods", "2010,5,26,4,15", &param6};

 // Missing 'id' parameter
  	param2.next = NULL;
    checkAlertUpdateMissingArg(tc, param7);
	param2.next = &param1;

 // Missing 'name' param
 	param3.next = &param1;
 	checkAlertUpdateMissingArg(tc, param7);
 	param3.next = &param2;

 // Missing 'active' param
 	param4.next = &param2;
 	checkAlertUpdateMissingArg(tc, param7);
 	param4.next = &param3;

 // Missing 'direction' param
 	param5.next = &param3;
 	checkAlertUpdateMissingArg(tc, param7);
 	param5.next = &param4;

 // Missing 'amount' param
 	param6.next = &param4;
 	checkAlertUpdateMissingArg(tc, param7);
 	param6.next = &param5;

 // Missing 'bound' param
 	param7.next = &param5;
 	checkAlertUpdateMissingArg(tc, param7);
 	param7.next = &param6;

 // Missing 'periods' param
 	checkAlertUpdateMissingArg(tc, param6);
}

void testAlertUpdateOk(CuTest *tc) {
	emptyDb();
	struct Alert* alert1 = allocAlert();
    alert1->active    = 1;
    alert1->direction = 1;
    alert1->amount    = 100000000000;
    alert1->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");

    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);

	struct Alert* alert2 = allocAlert();
    alert2->active    = 0;
    alert2->direction = 2;
    alert2->amount    = 200000000000;
    alert2->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");

    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);

    struct Alert* alert = getAlerts();
    CuAssertIntEquals(tc, alert->id, 1);
    CuAssertIntEquals(tc, alert->next->id, 2);
    CuAssertTrue(tc, alert->next->next == NULL);

 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"name", "newname", &param1};
 	struct NameValuePair param3 = {"active", "0", &param2};
 	struct NameValuePair param4 = {"direction", "2", &param3};
 	struct NameValuePair param5 = {"amount", "200", &param4};
 	struct NameValuePair param6 = {"bound", "['2009','4','25','3','14']", &param5};
 	struct NameValuePair param7 = {"periods", "[['2011','6','27','5','16']]", &param6};
 	struct NameValuePair param8 = {"action", "update", &param7};

    struct Request req = {"GET", "/alert", &param8, NULL};

    int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, TRUE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
		"HTTP/1.0 200 OK" HTTP_EOL
		"Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "{}"
    , result);

    alert = getAlerts();
    CuAssertIntEquals(tc, 2, alert->id);

	alert = alert->next;
    CuAssertIntEquals(tc, 1, alert->id);
    CuAssertStrEquals(tc, "newname", alert->name);
    CuAssertIntEquals(tc, 0, alert->active);
    CuAssertIntEquals(tc, 2, alert->direction);
    CuAssertIntEquals(tc, 200, alert->amount);

    struct DateCriteria* bound = alert->bound;
    checkDateCriteriaPart(tc, bound->year,    FALSE, 2009, 2009, FALSE);
	checkDateCriteriaPart(tc, bound->month,   FALSE, 4, 4, FALSE);
	checkDateCriteriaPart(tc, bound->day,     FALSE, 25, 25, FALSE);
	checkDateCriteriaPart(tc, bound->weekday, FALSE, 3, 3, FALSE);
	checkDateCriteriaPart(tc, bound->hour,    FALSE, 14, 14, FALSE);

	struct DateCriteria* period = alert->periods;
    checkDateCriteriaPart(tc, period->year,    FALSE, 2011, 2011, FALSE);
	checkDateCriteriaPart(tc, period->month,   FALSE, 6, 6, FALSE);
	checkDateCriteriaPart(tc, period->day,     FALSE, 27, 27, FALSE);
	checkDateCriteriaPart(tc, period->weekday, FALSE, 5, 5, FALSE);
	checkDateCriteriaPart(tc, period->hour,    FALSE, 16, 16, FALSE);

	CuAssertTrue(tc, period->next == NULL);
}

void testAlertUpdateForbidden(CuTest *tc) {
	emptyDb();
	struct Alert* alert1 = allocAlert();
    alert1->active    = 1;
    alert1->direction = 1;
    alert1->amount    = 100000000000;
    alert1->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert1, "alert1");

    struct DateCriteria* period1 = makeDateCriteria("*", "*", "*", "5,6", "0-6");
    struct DateCriteria* period2 = makeDateCriteria("*", "*", "*", "0-4", "6-12");
    period1->next = period2;
    alert1->periods = period1;
	addAlert(alert1);

	struct Alert* alert2 = allocAlert();
    alert2->active    = 0;
    alert2->direction = 2;
    alert2->amount    = 200000000000;
    alert2->bound     = makeDateCriteria("2010", "5", "26", "4", "15");
    setAlertName(alert2, "alert2");

    struct DateCriteria* period3 = makeDateCriteria("*", "*", "*", "2,8", "5");
    struct DateCriteria* period4 = makeDateCriteria("*", "*", "*", "1", "*");
    period3->next = period4;
    alert2->periods = period3;
    addAlert(alert2);

    struct Alert* alert = getAlerts();
    CuAssertIntEquals(tc, alert->id, 1);
    CuAssertIntEquals(tc, alert->next->id, 2);
    CuAssertTrue(tc, alert->next->next == NULL);

 	struct NameValuePair param1 = {"id", "1", NULL};
 	struct NameValuePair param2 = {"name", "newname", &param1};
 	struct NameValuePair param3 = {"active", "0", &param2};
 	struct NameValuePair param4 = {"direction", "2", &param3};
 	struct NameValuePair param5 = {"amount", "200", &param4};
 	struct NameValuePair param6 = {"bound", "['2009','4','25','3','14']", &param5};
 	struct NameValuePair param7 = {"periods", "[['2011','6','27','5','16']]", &param6};
 	struct NameValuePair param8 = {"action", "update", &param7};

    struct Request req = {"GET", "/alert", &param8, NULL};

    int tmpFd = makeTmpFile();
    processAlertRequest(tmpFd, &req, FALSE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
		"HTTP/1.0 403 Forbidden" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
    , result);

    alert = getAlerts();
    CuAssertIntEquals(tc, alert->id, 1);
    CuAssertIntEquals(tc, alert->next->id, 2);
    CuAssertTrue(tc, alert->next->next == NULL);
}

void testProcessAlertStatus(CuTest *tc) {
	emptyDb();

	struct Alert* alert1 = allocAlert();
    alert1->active    = 1;
    alert1->direction = 1;
    alert1->amount    = 1000000;
    alert1->bound     = makeDateCriteria("2009", "1", "1", "4", "1");
    setAlertName(alert1, "alert1");

    struct DateCriteria* period = makeDateCriteria("*", "*", "*", "*", "*");
    alert1->periods = period;

	addAlert(alert1);

 	struct NameValuePair param = {"action", "status", NULL};

    struct Request req = {"GET", "/alert", &param, NULL};
	addDbRow(makeTs("2009-11-01 00:00:00"), 3600, NULL, 200000, 300000, NULL);

    int tmpFd = makeTmpFile();
    processAlertStatus(tmpFd, &req, FALSE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
		"HTTP/1.0 200 OK" HTTP_EOL
		"Content-Type: application/json" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "[{\"id\" : 1,\"name\" : \"alert1\",\"current\" : 200000,\"limit\" : 1000000}]"
    , result);

}

CuSuite* handleAlertGetSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testAlertNoAction);
    SUITE_ADD_TEST(suite, testAlertListNone);
    SUITE_ADD_TEST(suite, testAlertList);
    SUITE_ADD_TEST(suite, testAlertDeleteOk);
    SUITE_ADD_TEST(suite, testAlertDeleteForbidden);
    SUITE_ADD_TEST(suite, testAlertUpdateMissingArgs);
    SUITE_ADD_TEST(suite, testAlertUpdateOk);
    SUITE_ADD_TEST(suite, testAlertUpdateForbidden);
    SUITE_ADD_TEST(suite, testProcessAlertStatus);

    return suite;
}
