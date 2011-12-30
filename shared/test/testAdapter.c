#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#ifndef _WIN32
#include <netinet/in.h>
#endif

/*
Contains unit tests for the 'adapter' module.
*/

#define ADAPTER_NAME "adapter1"
#define ADAPTER_DESC "Adapter 1"

void testAllocAdapter(void **state) { 
    char* name = ADAPTER_NAME;
    char* desc = ADAPTER_DESC;
    
    #ifdef _WIN32
	    struct sockaddr ip2 = {1, "005678"}; // IP 53.54.55.56
    	struct pcap_addr addr2 = {NULL, &ip2, NULL, NULL, NULL};

	    struct sockaddr ip1 = {1, "001234"}; // IP 49.50.51.52
    	struct pcap_addr addr1 = {&addr2, &ip1, NULL, NULL, NULL};
	#else
	    struct sockaddr ip2 = {16, AF_INET, "005678"}; // IP 53.54.55.56
	    struct pcap_addr addr2 = {NULL, &ip2, NULL, NULL, NULL};

    	struct sockaddr ip1 = {16, AF_INET, "001234"}; // IP 49.50.51.52
	    struct pcap_addr addr1 = {&addr2, &ip1, NULL, NULL, NULL};

	#endif
    
    struct pcap_if pcapif = {NULL, name, desc, &addr1, 0};
    struct Adapter* adapter = allocAdapter(&pcapif);
    
    assert_string_equal(name, adapter->name);
    assert_int_not_equal(name, adapter->name); // string is copied
    
    assert_string_equal("49.50.51.52 or 53.54.55.56", adapter->ips);
    
    assert_true(adapter->next == NULL);
    assert_true(adapter->next == NULL);
    assert_true(adapter->total == NULL);
    
    freeAdapters(adapter);
} 

void testFreeAdapters(void **state) { 
    struct sockaddr ip = {1, "001234"};
    struct pcap_addr addr = {NULL, &ip, NULL, NULL, NULL};
    
    struct pcap_if pcapif = {NULL, ADAPTER_NAME, ADAPTER_DESC, &addr, 0};
    
    struct Adapter* adapter1 = allocAdapter(&pcapif);
    struct Adapter* adapter2 = allocAdapter(&pcapif);
    struct Adapter* adapter3 = allocAdapter(&pcapif);
    
    adapter1->next = adapter2;
    adapter2->next = adapter3;
    
    freeAdapters(adapter1);
} 

void testAppendAdapter(void **state) { 
    struct sockaddr ip = {1, "001234"};
    struct pcap_addr addr = {NULL, &ip, NULL, NULL, NULL};
    
    struct pcap_if pcapif = {NULL, ADAPTER_NAME, ADAPTER_DESC, &addr, 0};
    
    struct Adapter* adapter0 = NULL;
    struct Adapter* adapter1 = allocAdapter(&pcapif);
    struct Adapter* adapter2 = allocAdapter(&pcapif);
    struct Adapter* adapter3 = allocAdapter(&pcapif);

    appendAdapter(&adapter0, adapter1);
    assert_int_equal(adapter0, adapter1);   

    appendAdapter(&adapter0, adapter2);
    assert_int_equal(adapter1->next, adapter2); 
    
    appendAdapter(&adapter1, adapter3);
    assert_int_equal(adapter2->next, adapter3); 

    freeAdapters(adapter1);
} 
