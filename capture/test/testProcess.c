#ifdef _WIN32
    #define __USE_MINGW_ANSI_STDIO 1
#endif
#define HAVE_REMOTE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "capture.h"
#include <pcap/pcap.h>
#include "pcap.h"
#include "remote-ext.h"
#include <setjmp.h>
#include <cmockery.h> 
#include <test.h> 

/*
Contains unit tests for the process module.
*/
static void testProcess(void** state, int promiscFlag);

void setupForProcessTest(void** state){
    setupTestDb(state);
    setConfigIntValue(CONFIG_DB_VERSION, DB_VERSION);
    setConfigIntValue(CONFIG_CAP_KEEP_SEC_LIMIT, 3600);   
    setConfigIntValue(CONFIG_CAP_KEEP_MIN_LIMIT, 86400);   
    setConfigIntValue(CONFIG_CAP_COMPRESS_INTERVAL, 3600);
}

void teardownForProcessTest(void** state){
    tearDownTestDb(state);
}

void testProcessNonPromisc(void** state){
    setupForProcessTest(0);
    setConfigIntValue(CONFIG_CAP_PROMISCUOUS, 0);
    testProcess(state, 0);
    teardownForProcessTest(0);
}
void testProcessPromisc(void** state){
    setupForProcessTest(0);
    setConfigIntValue(CONFIG_CAP_PROMISCUOUS, 1);
    testProcess(state, 1);
    teardownForProcessTest(0);
}

static void testProcess(void** state, int promiscFlag){
    char* filter1 = "port 80";
    char* filter2 = "port 90";
    addFilterRow(1, "Filter 1", "f1", filter1, NULL);
    addFilterRow(2, "Filter 2", "f2", filter2, NULL);
    
    int pcapOpenHandle1 = 100;
    int pcapOpenHandle2 = 101;
    int pcapOpenHandle3 = 102;
    int pcapOpenHandle4 = 103;
    
    #ifdef _WIN32
	    will_return(mockPcap_open, pcapOpenHandle1);
    	will_return(mockPcap_open, pcapOpenHandle2);
    	will_return(mockPcap_open, pcapOpenHandle3);
    	will_return(mockPcap_open, pcapOpenHandle4);
    #else
	    will_return(mockPcap_open_live, pcapOpenHandle1);
    	will_return(mockPcap_open_live, pcapOpenHandle2);
    	will_return(mockPcap_open_live, pcapOpenHandle3);
    	will_return(mockPcap_open_live, pcapOpenHandle4);
	#endif
    expect_call(mockOpenDb);
    //expect_call(_compressDb);
    
    #ifdef _WIN32
	    expect_value(mockPcap_open, flags, promiscFlag);
    	expect_value(mockPcap_open, flags, promiscFlag);
	    expect_value(mockPcap_open, flags, promiscFlag);
    	expect_value(mockPcap_open, flags, promiscFlag);
    #else
	    expect_value(mockPcap_open_live, flags, promiscFlag);
    	expect_value(mockPcap_open_live, flags, promiscFlag);
	    expect_value(mockPcap_open_live, flags, promiscFlag);
    	expect_value(mockPcap_open_live, flags, promiscFlag);
    #endif
    
	#ifdef STATS_MODE
		expect_value(mockPcap_setnonblock, h, pcapOpenHandle1);
		expect_value(mockPcap_setnonblock, h, pcapOpenHandle2);
		expect_value(mockPcap_setnonblock, h, pcapOpenHandle3);
		expect_value(mockPcap_setnonblock, h, pcapOpenHandle4);
	#endif
    
    expect_value(mockPcap_compile, h, pcapOpenHandle1);
    expect_value(mockPcap_compile, h, pcapOpenHandle2);
    expect_value(mockPcap_compile, h, pcapOpenHandle3);
    expect_value(mockPcap_compile, h, pcapOpenHandle4);
    
    expect_string(mockPcap_compile, c, filter1);
    expect_string(mockPcap_compile, c, filter2);
    expect_string(mockPcap_compile, c, filter1);
    expect_string(mockPcap_compile, c, filter2);
                                      
    expect_value(mockPcap_setfilter, h, pcapOpenHandle1);
    expect_value(mockPcap_setfilter, h, pcapOpenHandle2);
    expect_value(mockPcap_setfilter, h, pcapOpenHandle3);
    expect_value(mockPcap_setfilter, h, pcapOpenHandle4);

	#ifdef STATS_MODE 
		expect_value(mockPcap_setmode, h, pcapOpenHandle1);
		expect_value(mockPcap_setmode, h, pcapOpenHandle2);
		expect_value(mockPcap_setmode, h, pcapOpenHandle3);
		expect_value(mockPcap_setmode, h, pcapOpenHandle4);
	#endif

    expect_value(mockPcap_close, h, pcapOpenHandle1);
    expect_value(mockPcap_close, h, pcapOpenHandle2);
    expect_value(mockPcap_close, h, pcapOpenHandle3);
    expect_value(mockPcap_close, h, pcapOpenHandle4);

    expect_value(mockPcap_dispatch, h, pcapOpenHandle1);
    expect_value(mockPcap_dispatch, h, pcapOpenHandle2);
    expect_value(mockPcap_dispatch, h, pcapOpenHandle3);
    expect_value(mockPcap_dispatch, h, pcapOpenHandle4);

    expect_call(mockCloseDb);
    setupCapture();
    processCapture();
    shutdownCapture();
    freeStmtList();
}

