#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "client.h"
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

void testParseRequest(void** state) {
	struct Request* request;

 // Minimal path, no parameters
	request = buildRequest("/", NULL);
	assert_string_equal("GET", request->method);
	assert_string_equal("/", request->path);
	assert_true(request->params == NULL);
	freeRequest(request);

 // Non-minimal path, no parameters
	request = buildRequest("/pathOnly", NULL);
	assert_string_equal("/pathOnly", request->path);
	assert_true(request->params == NULL);
	freeRequest(request);

 // Non-minimal path, no parameters
	request = buildRequest("/pathOnly?", NULL);
	assert_string_equal("/pathOnly", request->path);
	assert_true(request->params == NULL);
	freeRequest(request);

 // Parameter name with no value
	request = buildRequest("/path?paramWithNoValue", NULL);
	assert_string_equal("/path", request->path);
	assert_true(request->params == NULL);
	freeRequest(request);

 // Multiple parameters
	request = buildRequest("/path?a=b&c=d", NULL);
	assert_string_equal("/path", request->path);

	struct NameValuePair* param;

	param = request->params;
	assert_string_equal("a", param->name);
	assert_string_equal("b", param->value);

	param = param->next;
	assert_string_equal("c", param->name);
	assert_string_equal("d", param->value);
	assert_true(param->next == NULL);
	freeRequest(request);
	
 // Multiple parameters, first has empty value
	request = buildRequest("/path?a=&c=d", NULL);
	assert_string_equal("/path", request->path);

	param = request->params;
	assert_string_equal("a", param->name);
	assert_string_equal("", param->value);

	param = param->next;
	assert_string_equal("c", param->name);
	assert_string_equal("d", param->value);
	assert_true(param->next == NULL);
	freeRequest(request);
	
 // Multiple parameters, last has empty value
	request = buildRequest("/path?a=b&c=", NULL);
	assert_string_equal("/path", request->path);

	param = request->params;
	assert_string_equal("a", param->name);
	assert_string_equal("b", param->value);

	param = param->next;
	assert_string_equal("c", param->name);
	assert_string_equal("", param->value);
	assert_true(param->next == NULL);

	freeRequest(request);
}

