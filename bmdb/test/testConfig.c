#define _GNU_SOURCE
#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include <stdio.h>
#include "bmdb.h"
#include "string.h"
#include "test.h"

/*
Contains unit tests for the bmdb config module.
*/

void testConfigDump(void** state){
    addConfigRow("key1", "value1");
    addConfigRow("key2", "value2");

	expect_string(printf_output, msg, INFO_DUMPING_CONFIG EOL); 
	expect_string(printf_output, msg, "key1"); 
	expect_string(printf_output, msg, "=value1" EOL); 
	expect_string(printf_output, msg, "key2"); 
	expect_string(printf_output, msg, "=value2" EOL); 

    doListConfig(NULL,0,0);
    freeStmtList();
}

void testConfigUpdate(void** state){
    addConfigRow("key1", "value1");
    addConfigRow("key2", "value2");

	char* args[] = {"key1", "value3"};
	expect_string(printf_output, msg, "Config value 'key1' set to 'value3'." EOL); 
	
	doSetConfig(NULL, 2, args);

	expect_string(printf_output, msg, INFO_DUMPING_CONFIG EOL); 
	expect_string(printf_output, msg, "key1" ); 
	expect_string(printf_output, msg, "=value3" EOL); 
	expect_string(printf_output, msg, "key2" ); 
	expect_string(printf_output, msg, "=value2" EOL); 	

    doListConfig(NULL, 0, 0);
    freeStmtList();
}

void testConfigDelete(void** state){
    addConfigRow("key1", "value1");
    addConfigRow("key2", "value2");

	char* args[] = {"key1"};
	expect_string(printf_output, msg, "Config value 'key1' was removed." EOL); 
	doRmConfig(NULL,1, args);

	expect_string(printf_output, msg, INFO_DUMPING_CONFIG EOL); 
	expect_string(printf_output, msg, "key2" ); 
	expect_string(printf_output, msg, "=value2" EOL); 
    doListConfig(NULL,0, 0);
    freeStmtList();
}
