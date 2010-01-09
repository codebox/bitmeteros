/*
 * BitMeterOS v0.3.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
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
 *
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include "CuTest.h"
#include "bmws.h"

/*
Contains unit tests for the httpRequest module.
*/

static struct Request* buildRequest(char* path, char* headers){
	char txt[512];
	if (headers != NULL){
		sprintf(txt, "GET %s HTTP/1.1" HTTP_EOL "%s" HTTP_EOL HTTP_EOL, path, headers);
	} else {
		sprintf(txt, "GET %s HTTP/1.1" HTTP_EOL HTTP_EOL, path);
	}
	return parseRequest(txt);
}

void testParseRequest(CuTest *tc) {
	struct Request* request;

 // Minimal path, no parameters or headers
	request = buildRequest("/", NULL);
	CuAssertStrEquals(tc, "GET", request->method);
	CuAssertStrEquals(tc, "/", request->path);
	CuAssertTrue(tc, request->params == NULL);
	freeRequest(request);

 // Non-minimal path, no parameters or headers
	request = buildRequest("/pathOnly", NULL);
	CuAssertStrEquals(tc, "/pathOnly", request->path);
	CuAssertTrue(tc, request->params == NULL);
	freeRequest(request);

 // Non-minimal path, no parameters or headers
	request = buildRequest("/pathOnly?", NULL);
	CuAssertStrEquals(tc, "/pathOnly", request->path);
	CuAssertTrue(tc, request->params == NULL);
	freeRequest(request);

 // Parameter name with no value
	request = buildRequest("/path?paramWithNoValue", NULL);
	CuAssertStrEquals(tc, "/path", request->path);
	CuAssertTrue(tc, request->params == NULL);
	freeRequest(request);

 // Multiple parameters, no headers
	request = buildRequest("/path?a=b&c=d", NULL);
	CuAssertStrEquals(tc, "/path", request->path);

	struct NameValuePair* param;

	param = request->params;
	CuAssertStrEquals(tc, "a", param->name);
	CuAssertStrEquals(tc, "b", param->value);

	param = param->next;
	CuAssertStrEquals(tc, "c", param->name);
	CuAssertStrEquals(tc, "d", param->value);
	CuAssertTrue(tc, param->next == NULL);

	freeRequest(request);

 // Multiple parameters and headers
	/*request = buildRequest("/path?e=f&g=h", "h1: v1" HTTP_EOL "h2: v2");
	CuAssertStrEquals(tc, "/path", request->path);

	param = request->params;
	CuAssertStrEquals(tc, "e", param->name);
	CuAssertStrEquals(tc, "f", param->value);

	param = param->next;
	CuAssertStrEquals(tc, "g", param->name);
	CuAssertStrEquals(tc, "h", param->value);
	CuAssertTrue(tc, param->next == NULL);

	struct NameValuePair* header = request->headers;
	CuAssertStrEquals(tc, "h1", header->name);
	CuAssertStrEquals(tc, "v1", header->value);

	header = header->next;
	CuAssertStrEquals(tc, "h2", header->name);
	CuAssertStrEquals(tc, "v2", header->value);
	CuAssertTrue(tc, header->next == NULL);

	freeRequest(request);

 // Minimal path, no parameters, multiple headers
	request = buildRequest("/", "h3: v3" HTTP_EOL "h4: v4");
	CuAssertStrEquals(tc, "/", request->path);

	CuAssertTrue(tc, request->params == NULL);

	header = request->headers;
	CuAssertStrEquals(tc, "h3", header->name);
	CuAssertStrEquals(tc, "v3", header->value);

	header = header->next;
	CuAssertStrEquals(tc, "h4", header->name);
	CuAssertStrEquals(tc, "v4", header->value);
	CuAssertTrue(tc, header->next == NULL);

	freeRequest(request);*/
}

void testGetValueForName(CuTest *tc) {
 // Checks that the getValueForName function works correctly
	CuAssertTrue(tc, getValueForName("p", NULL, NULL) == (char*)NULL);

 // A single pair
	struct NameValuePair* param1 = malloc(sizeof(struct NameValuePair));
	param1->name  = "a";
	param1->value = "b";
	param1->next = NULL;
	CuAssertTrue(tc, getValueForName("p", param1, NULL) == NULL);
	CuAssertStrEquals(tc, "x", getValueForName("p", param1, "x"));
	CuAssertStrEquals(tc, "b", getValueForName("a", param1, NULL));
	CuAssertStrEquals(tc, "b", getValueForName("a", param1, "x"));

 // Multiple pairs
	struct NameValuePair* param2 = malloc(sizeof(struct NameValuePair));
	param1->next = param2;
	param2->name  = "c";
	param2->value = "d";
	param2->next = NULL;
	CuAssertTrue(tc, getValueForName("p", param1, NULL) == NULL);
	CuAssertStrEquals(tc, "x", getValueForName("p", param1, "x"));
	CuAssertStrEquals(tc, "b", getValueForName("a", param1, NULL));
	CuAssertStrEquals(tc, "b", getValueForName("a", param1, "x"));
	CuAssertStrEquals(tc, "d", getValueForName("c", param1, NULL));
	CuAssertStrEquals(tc, "d", getValueForName("c", param1, "x"));

	//freeNameValuePairs(param1);
}

void testGetValueNumForName(CuTest *tc) {
 // Checks that the getValueNumForName function works correctly
	CuAssertIntEquals(tc, -1, getValueNumForName("p", NULL, -1));

 // A single pair
	struct NameValuePair* param1 = malloc(sizeof(struct NameValuePair));
	param1->name  = "a";
	param1->value = "7";
	param1->next = NULL;
	CuAssertIntEquals(tc, -1, getValueNumForName("p", param1, -1));
	CuAssertIntEquals(tc,  7, getValueNumForName("a", param1, -1));

 // Multiple pairs
	struct NameValuePair* param2 = malloc(sizeof(struct NameValuePair));
	param1->next = param2;
	param2->name  = "c";
	param2->value = "0";
	param2->next = NULL;
	CuAssertIntEquals(tc, -1, getValueNumForName("p", param1, -1));
	CuAssertIntEquals(tc,  7, getValueNumForName("a", param1, -1));
	CuAssertIntEquals(tc,  0, getValueNumForName("c", param1, -1));

	struct NameValuePair* param3 = malloc(sizeof(struct NameValuePair));
	param2->next = param3;
	param3->name  = "x";
	param3->value = "notanumber";
	param3->next = NULL;
	CuAssertIntEquals(tc, -1, getValueNumForName("p", param1, -1));
	CuAssertIntEquals(tc,  7, getValueNumForName("a", param1, -1));
	CuAssertIntEquals(tc,  0, getValueNumForName("c", param1, -1));
	CuAssertIntEquals(tc, -1, getValueNumForName("x", param1, -1));

	//freeNameValuePairs(param1);
}

CuSuite* httpRequestGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testParseRequest);
    SUITE_ADD_TEST(suite, testGetValueForName);
    SUITE_ADD_TEST(suite, testGetValueNumForName);
    return suite;
}
