#include <stdio.h>
#include "common.h"
#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"
#include "client.h"
#include "bmws.h"

/*
Contains unit tests for the handleFile module.
*/

size_t _fread(void* buffer, size_t a, size_t b, FILE* c){
	char* txt = mock();
	strcpy(buffer, txt);
	return strlen(txt);
}
void _writeData(SOCKET fd, char* data, int len){
	check_expected(data);
}

void setupTestForHandleFile(void** state){
 	struct HandleFileCalls calls = {&_fread, &_writeData};
	mockHandleFileCalls = calls;
}

void tearDownTestForHandleFile(void** state){
}

void testCharSubstitution(void** state) {
    struct NameValuePair p1 = {"v1", "1", NULL};
    struct NameValuePair p2 = {"v2", "2", &p1};
    
    will_return(_fread, "val1=<!--[v1]--> val2=<!--[v2]--> val1=<!--[v1]-->");
	expect_string(_writeData, data, "val1=1 val2=2 val1=1");
	
    doSubs(0, 0, &p2);
}

void testGetMimeTypeForFile(void** state){
	struct MimeType* mimeType = getMimeTypeForFile("file.html");
	assert_string_equal("html", mimeType->fileExt);
	assert_string_equal("text/html", mimeType->contentType);
	
	mimeType = getMimeTypeForFile("file.html.css");
	assert_string_equal("css", mimeType->fileExt);
	assert_string_equal("text/css", mimeType->contentType);
	
	mimeType = getMimeTypeForFile("file.other");
	assert_string_equal("bin", mimeType->fileExt);
	assert_string_equal("application/octet-stream", mimeType->contentType);
}
