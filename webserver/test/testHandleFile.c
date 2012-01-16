#include <stdio.h>
#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"
#include "common.h"
#include "client.h"
#include "bmws.h"

/*
Contains unit tests for the handleFile module.
*/

void testCharSubstitution(void** state) {
    struct NameValuePair p1 = {"v1", "1", NULL};
    struct NameValuePair p2 = {"v2", "2", &p1};
    
    will_return(mockFread, "val1=<!--[v1]--> val2=<!--[v2]--> val1=<!--[v1]-->");
    expect_string(mockWriteData, data, "val1=1 val2=2 val1=1");
    
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
