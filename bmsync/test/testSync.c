#define _GNU_SOURCE
#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "string.h"
#include "bmsync.h"
#include "bmws.h"

/*
Contains unit tests for the sync.c module.
*/

void testGetMaxTsForHost(void** state){
	addFilterRow(1, "filter1", "f1", "x1", "host1");
	addFilterRow(2, "filter2", "f2", "x2", NULL);
	addDbRow(100, 1, 1, 1);
	addDbRow(101, 1, 1, 1);
	addDbRow(101, 1, 1, 2);
	addDbRow(102, 1, 1, 2);
	
	assert_int_equal(101, getMaxTsForHost("host1"));
	assert_int_equal(0,   getMaxTsForHost("host2"));
	assert_int_equal(102, getMaxTsForHost(NULL));
	
	freeStmtList();
}

void testParseFilterRow(void** state){
	struct Filter* filter;
	
	filter = parseFilterRow("filter:1,name,desc,expr", "host");
	checkFilter(filter, 1, "desc", "name", "expr", "host");
	freeFilters(filter);

	filter = parseFilterRow("filter:1,name,desc,(e,x,p,r)", NULL);
	checkFilter(filter, 1, "desc", "name", "(e,x,p,r)", NULL);
	freeFilters(filter);

	filter = parseFilterRow("something else", NULL);
	assert_true(filter == NULL);
}

void testParseDataRow(void** state){
	struct Data* data;
	
	data = parseDataRow("1,2,3,4");
	checkData(data,1,2,3,4);
	freeData(data);
	
	data = parseDataRow("something else");
	assert_true(data == NULL);
}

void testStartsWith(void** state){
	assert_true(startsWith("abc", "a"));
	assert_true(startsWith("abc", "abc"));
	assert_true(startsWith("abc", ""));
	assert_true(startsWith("", ""));
	
	assert_false(startsWith("abc", "abcd"));	
	assert_false(startsWith("", "a"));
}

void testGetLocalId(void** state){
//int getLocalId(struct RemoteFilter* remoteFilter, int filterId)	
	struct RemoteFilter rf1 = {100, 1, NULL};
	int localId;
	
	localId = getLocalId(&rf1, 100);
	assert_int_equal(localId, 1);
	
	localId = getLocalId(&rf1, 101);
	assert_int_equal(localId, 0);
	
	struct RemoteFilter rf2 = {200, 2, &rf1};
	struct RemoteFilter rf3 = {300, 3, &rf2};
	
	localId = getLocalId(&rf3, 100);
	assert_int_equal(localId, 1);
	
	localId = getLocalId(&rf3, 300);
	assert_int_equal(localId, 3);
	
	localId = getLocalId(&rf3, 101);
	assert_int_equal(localId, 0);
}

void testGetLocalFilter(void** state){
	int localId;
	
	addFilterRow(1, "filter1", "f1", "x1", "host1");
	addFilterRow(2, "filter2", "f2", "x2", NULL);
	addFilterRow(3, "filter3", "f3", "x3", "host2");
	addFilterRow(4, "filter4", "f4", "x4", "host2");
	
	struct Filter rf1 = {100, "remotefilter3", "f3", "remotex3", "host2"};
	localId = getLocalFilter(&rf1);
	assert_int_equal(3, localId);
	
	struct Filter rf2 = {200, "remotefilter1", "f1", "remotex1", "host1"};
	localId = getLocalFilter(&rf2);
	assert_int_equal(1, localId);
	
 // Creates a new local filter
	struct Filter rf3 = {300, "remotefilter1", "f1", "remotex1", "host3"};
	localId = getLocalFilter(&rf3);
	assert_int_equal(5, localId);
	
	struct Filter* filter = getFilter("f1", "host3");
	checkFilter(filter, 5, "remotefilter1", "f1", "remotex1", "host3");
	freeFilters(filter);
	
	freeStmtList();
}

void testAppendRemoteFilter(void** state){
	struct RemoteFilter* rf0 = NULL;
	struct RemoteFilter rf1 = {100, 1, NULL};
	
	appendRemoteFilter(&rf0, &rf1);
	assert_int_equal(100, rf0->remoteId);
	assert_int_equal(1, rf0->localId);
	assert_true(rf0->next == NULL);
	
	struct RemoteFilter rf2 = {200, 2, NULL};
	appendRemoteFilter(&rf0, &rf2);
	
	assert_int_equal(100, rf0->remoteId);
	assert_int_equal(1, rf0->localId);
	assert_int_equal(200, rf0->next->remoteId);
	assert_int_equal(2, rf0->next->localId);
	assert_true(rf0->next->next == NULL);
}

void testRemoveDataForDeletedFiltersFromThisHost(void** state){
	struct RemoteFilter rf1 = {100, 1, NULL};
	struct RemoteFilter rf2 = {200, 2, &rf1};
	struct RemoteFilter rf3 = {500, 5, &rf2};
	
	addFilterRow(1, "filter1", "f1", "x1", "host1");
	addFilterRow(2, "filter2", "f2", "x2", "host1");
	addFilterRow(3, "filter3", "f3", "x3", NULL);
	addFilterRow(4, "filter4", "f4", "x4", "host2");
	addFilterRow(5, "filter5", "f5", "x5", "host1");
	
	addDbRow(100, 1, 1000, 5);
	addDbRow(100, 1, 1000, 1);
	addDbRow(101, 1, 1000, 5);
	addDbRow(101, 1, 1000, 3);
	addDbRow(102, 1, 1000, 4);
	addDbRow(103, 1, 1000, 5);

 // nothing should be removed at this stage	
	removeDataForDeletedFiltersFromThisHost("host1", &rf3);
	assert_int_equal(6, getRowCount("select * from data2"));
	                                         
 // again, nothing removed because no items for this host exist	                                         
	struct RemoteFilter rf4 = {600, 6, NULL};
	removeDataForDeletedFiltersFromThisHost("host3", &rf4);
	assert_int_equal(6, getRowCount("select * from data2"));

 // this time only 1 and 2 came through, so 5 must have been deleted
	removeDataForDeletedFiltersFromThisHost("host1", &rf2);

	struct Data d1 = {100, 1, 1000, 1, NULL};
	struct Data d2 = {101, 1, 1000, 3, &d1};
	struct Data d3 = {102, 1, 1000, 4, &d2};
	checkTableContents(&d3);

 // remove the last filter for this host
	struct RemoteFilter rf5 = {200, 2, NULL};
	removeDataForDeletedFiltersFromThisHost("host1", &rf5);
	
	struct Data d4 = {101, 1, 1000, 3, NULL};
	struct Data d5 = {102, 1, 1000, 4, &d4};
	checkTableContents(&d5);
	
	freeStmtList();
}
static char* getRecvLine(){
	return (char*)mock();
}
static int _recv(SOCKET fd, char* buffer, int a, int b){
	char* txt = getRecvLine();
	if (txt != NULL){
		buffer[0] = txt[0];	
		buffer[1] = 0;
		return 1;
	} else {
		return 0;
	}
}
static int _send(SOCKET fd, char* buffer, int a, int b){
	check_expected(buffer);
}
static void setupMocks(){
	struct SyncCalls calls = {&_recv, &_send};	
	mockSyncCalls = calls;
}
static void setupRecvReturns(char* txt){
	if (txt == NULL){
		will_return(getRecvLine, NULL);
	} else {
		while(*txt != 0){
			will_return(getRecvLine, (txt++));
		}	
	}
}
void testReadLine(void** state){
	setupMocks();                 
	char buffer[200];
	
	setupRecvReturns("ABCD\r\nXYZ");
	readLine(1, buffer);
	
	assert_string_equal("ABCD\r\n", buffer);
 // drain out unread characters 'XYZ'
	getRecvLine();
	getRecvLine();
	getRecvLine();
	
	setupRecvReturns("\r\n");  
	readLine(1, buffer);
	assert_string_equal("\r\n", buffer);

	char overMaxLen[MAX_LINE_LEN + 2];
	int i;
	for(i=0; i<MAX_LINE_LEN+1; i++){
		overMaxLen[i] = 'x';
	}
	overMaxLen[MAX_LINE_LEN + 1] = 0;
	
	setupRecvReturns(overMaxLen);  
	readLine(1, buffer);
	
	char truncatedToMaxLen[MAX_LINE_LEN+1];
	for(i=0; i<MAX_LINE_LEN; i++){
		truncatedToMaxLen[i] = 'x';
	}
	truncatedToMaxLen[MAX_LINE_LEN] = 0;
	assert_string_equal(truncatedToMaxLen, buffer);
}

void testHttpHeadersOk(void** state){
	setupMocks();
	
 // Everything is ok
	setupRecvReturns("HTTP 200 OK\r\n");
	setupRecvReturns("Content-Type: application/vnd.codebox.bitmeter-sync\r\n");
	setupRecvReturns("SomeOther: stuff\r\n");
	setupRecvReturns("\r\n");
	assert_int_equal(SUCCESS, httpHeadersOk(1));
    
 // HTTP error code
	setupRecvReturns("HTTP 500 OK\r\n");
	assert_int_equal(FAIL, httpHeadersOk(1));
    
 // Bad content type
	setupRecvReturns("HTTP 200 OK\r\n");
	setupRecvReturns("Content-Type: text/html\r\n");
	assert_int_equal(FAIL, httpHeadersOk(1));
}

void testParseDataOk(void** state){
	setupMocks();
	
 // Everything is ok
	setupRecvReturns("HTTP 200 OK\r\n");
	setupRecvReturns("Content-Type: application/vnd.codebox.bitmeter-sync\r\n");
	setupRecvReturns("SomeOther: stuff\r\n");
	setupRecvReturns("\r\n");
	setupRecvReturns("filter:101,f1,filter 1,x1\r\n");
	setupRecvReturns("filter:102,f2,filter 2,x2\r\n");
	setupRecvReturns("1000,1,10,101\r\n");
	setupRecvReturns("1000,1,10,102\r\n");
	setupRecvReturns("1001,1,10,101\r\n");
	setupRecvReturns("1002,1,10,102\r\n");
	setupRecvReturns(NULL);
	
	int rc;
	int result = parseData(1, "host", &rc);
	
	assert_int_equal(SUCCESS, result);
	
	
	struct Filter* filters = readFilters();
	struct Filter* filter = filters;
	
	checkFilter(filter, 1, "filter 1", "f1", "x1", "host");
	filter = filter->next;
	checkFilter(filter, 2, "filter 2", "f2", "x2", "host");
	assert_true(filter->next == NULL);
	freeFilters(filters);
	
	struct Data d1 = {1000, 1, 10, 2, NULL};
	struct Data d2 = {1000, 1, 10, 1, &d1};
	struct Data d3 = {1001, 1, 10, 1, &d2};
	struct Data d4 = {1002, 1, 10, 2, &d3};
	checkTableContents(&d4);
	
	freeStmtList();
}

void testSendReqToDefaultPort(void** state){
	expect_string(_send, buffer, "GET /sync?ts=1000 HTTP/1.1\r\n");
	expect_string(_send, buffer, "Host: host");
	expect_string(_send, buffer, "\r\n\r\n");
	int result = sendRequest(1, 1000, "host", 80);
	assert_int_equal(SUCCESS, result);
}
void testSendReqToOtherPort(void** state){
	expect_string(_send, buffer, "GET /sync?ts=1000 HTTP/1.1\r\n");
	expect_string(_send, buffer, "Host: host:2605");
	expect_string(_send, buffer, "\r\n\r\n");
	int result = sendRequest(1, 1000, "host", 2605);
	assert_int_equal(SUCCESS, result);
}