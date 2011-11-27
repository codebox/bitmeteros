#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "client.h"
#include "test.h"
#include "bmws.h"

void testGetPortNoConfig(void** state){
    assert_int_equal(2605, getPort());
    freeStmtList();
}
void testGetPortConfigTooSmall(void** state){
    addConfigRow("web.port", "0");
    assert_int_equal(2605, getPort());
    freeStmtList();
}
void testGetPortConfigTooBig(void** state){
    addConfigRow("web.port", "70000");
    assert_int_equal(2605, getPort());
    freeStmtList();
}
void testGetPortConfigOk(void** state){
    addConfigRow("web.port", "5500");
    assert_int_equal(5500, getPort());
    freeStmtList();
}
void testReadDbConfig(void** state){
    struct WebConnectionConfig config;
    addConfigRow("db.version", "8");

 // No remote access allowed    
    addConfigRow("web.allow_remote", "0");
    expect_call(mockOpenDb);
    expect_call(mockCloseDb);
    config = readDbConfig();
    
    assert_int_equal(2605, config.port);
    assert_int_equal(0, config.allowRemoteConnect);
    assert_int_equal(0, config.allowRemoteAdmin);
    
 // Remote read-only access allowed
    addConfigRow("web.allow_remote", "1");
    expect_call(mockOpenDb);
    expect_call(mockCloseDb);
    config = readDbConfig();
    
    assert_int_equal(2605, config.port);
    assert_int_equal(1, config.allowRemoteConnect);
    assert_int_equal(0, config.allowRemoteAdmin);

 // Remote admin access allowed
    addConfigRow("web.allow_remote", "2");
    expect_call(mockOpenDb);
    expect_call(mockCloseDb);
    config = readDbConfig();
    
    assert_int_equal(2605, config.port);
    assert_int_equal(1, config.allowRemoteConnect);
    assert_int_equal(1, config.allowRemoteAdmin);

 // Non-standard port
    addConfigRow("web.port", "1234");
    expect_call(mockOpenDb);
    expect_call(mockCloseDb);
    config = readDbConfig();
    
    assert_int_equal(1234, config.port);
    
    freeStmtList();
}