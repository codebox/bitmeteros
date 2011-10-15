#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"

/*
Contains unit tests for the 'total' module.
*/

static int _pcap_close(pcap_t *h){
	check_expected(h);
	return 0;
}

void setupTestForTotal(void** state){
	struct TotalCalls _totalCalls = {&_pcap_close};
	mockTotalCalls = _totalCalls;
}

void teardownTestForTotal(void** state){
}

void testAllocTotal(void **state) { 
	struct Filter filter = {0, NULL, NULL, NULL, NULL, NULL};
	struct Total* total = allocTotal(&filter);
	
	assert_int_equal(0, total->count);
	assert_true(total->filter == &filter);
	assert_true(total->handle == NULL);
	assert_true(total->next == NULL);
	
	free(total);
} 

void testFreeTotals(void **state) { 
	struct Filter filter = {0, NULL, NULL, NULL, NULL, NULL};
	struct Total* total1 = allocTotal(&filter);
	total1->handle = 1;
	struct Total* total2 = allocTotal(&filter);
	total2->handle = 2;
	struct Total* total3 = allocTotal(&filter);
	total3->handle = 3;
	
	total1->next = total2;
	total2->next = total3;
	
	expect_value(_pcap_close, h, 1); 
	expect_value(_pcap_close, h, 2); 
	expect_value(_pcap_close, h, 3); 
	freeTotals(total1);
}

void testAppendTotals(void **state) { 
	struct Filter filter = {0, NULL, NULL, NULL, NULL, NULL};
	struct Total* total0 = NULL;
	struct Total* total1 = allocTotal(&filter);
	struct Total* total2 = allocTotal(&filter);
	struct Total* total3 = allocTotal(&filter);
	
	appendTotal(&total0, total1);
	assert_int_equal(total0, total1);
	assert_int_equal(total1->next, NULL);

	appendTotal(&total1, total2);
	assert_int_equal(total1->next, total2);
	assert_int_equal(total2->next, NULL);

	appendTotal(&total2, total3);
	assert_int_equal(total1->next, total2);
	assert_int_equal(total2->next, total3);
	assert_int_equal(total3->next, NULL);

	freeTotals(total1);
}