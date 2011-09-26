#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"

/*
Contains unit tests for the 'filter' module.
*/

#define FILTER_ID   1
#define FILTER_DESC "desc"
#define FILTER_NAME "name"
#define FILTER_EXPR "expr"
#define FILTER_HOST "host"
#define FILTER_IP   "1.2.3.4"

void testAllocFilter(void **state) { 
	struct Filter* filter = allocFilter(FILTER_ID, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	
	assert_int_equal(FILTER_ID, filter->id);
	assert_string_equal(FILTER_DESC, filter->desc);
	assert_string_equal(FILTER_NAME, filter->name);
	assert_string_equal(FILTER_EXPR, filter->expr);
	assert_string_equal(FILTER_HOST, filter->host);
	
	freeFilters(filter);
} 

void testAllocFilterWithNulls(void **state) { 
	struct Filter* filter = allocFilter(FILTER_ID, NULL, NULL, NULL, NULL);
	
	assert_int_equal(FILTER_ID, filter->id);
	assert_true(NULL == filter->desc);
	assert_true(NULL == filter->name);
	assert_true(NULL == filter->expr);
	assert_true(NULL == filter->host);
	
	freeFilters(filter);
} 

void testFreeFilter(void **state){
	struct Filter* filter1 = allocFilter(FILTER_ID, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	struct Filter* filter2 = allocFilter(FILTER_ID, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	struct Filter* filter3 = allocFilter(FILTER_ID, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	
	filter1->next = filter2;
	filter2->next = filter3;
	
	freeFilters(filter1);
}

void testGetFilterFromId(void **state){
	struct Filter* filter1 = allocFilter(1, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	struct Filter* filter2 = allocFilter(2, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	struct Filter* filter3 = allocFilter(3, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	
	filter1->next = filter2;
	filter2->next = filter3;

	struct Filter* result;
	
	result = getFilterFromId(filter1, 1);
	assert_int_equal(filter1, result);

	result = getFilterFromId(filter1, 2);
	assert_int_equal(filter2, result);

	result = getFilterFromId(filter1, 3);
	assert_int_equal(filter3, result);

	result = getFilterFromId(filter1, 4);
	assert_true(result == NULL);

	freeFilters(filter1);
}

void testGetFilterFromName(void **state){
	struct Filter* filter1 = allocFilter(FILTER_ID, FILTER_DESC, "A", FILTER_EXPR, FILTER_HOST);
	struct Filter* filter2 = allocFilter(FILTER_ID, FILTER_DESC, "B", FILTER_EXPR, FILTER_HOST);
	struct Filter* filter3 = allocFilter(FILTER_ID, FILTER_DESC, "C", FILTER_EXPR, FILTER_HOST);
	
	filter1->next = filter2;
	filter2->next = filter3;
	
	struct Filter* result;
	
	result = getFilterFromName(filter1, "A");
	assert_int_equal(filter1, result);

	result = getFilterFromName(filter1, "B");
	assert_int_equal(filter2, result);

	result = getFilterFromName(filter1, "C");
	assert_int_equal(filter3, result);

	result = getFilterFromName(filter1, "X");
	assert_true(result == NULL);
	
	freeFilters(filter1);
}

void testAppendFilter(void **state){
	struct Filter* filter0 = NULL;
	struct Filter* filter1 = allocFilter(1, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	struct Filter* filter2 = allocFilter(2, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);
	struct Filter* filter3 = allocFilter(3, FILTER_DESC, FILTER_NAME, FILTER_EXPR, FILTER_HOST);

	appendFilter(&filter0, filter1);
	assert_int_equal(filter0, filter1);
	assert_true(filter1->next == NULL);

	appendFilter(&filter0, filter2);
	assert_int_equal(filter1, filter0);
	assert_int_equal(filter1->next, filter2);
	assert_true(filter2->next == NULL);

	appendFilter(&filter1, filter3);
	assert_int_equal(filter1, filter0);
	assert_int_equal(filter1->next, filter2);
	assert_int_equal(filter2->next, filter3);
	assert_true(filter3->next == NULL);
	
	freeFilters(filter1);
}

void testGetTotalForFilter(void **state){
	struct Filter filter1 = {1, NULL, NULL, NULL, NULL, NULL};
	struct Filter filter2 = {2, NULL, NULL, NULL, NULL, NULL};
	struct Filter filter3 = {3, NULL, NULL, NULL, NULL, NULL};
	
	struct Total total31 = {1, &filter1, NULL, NULL};
	
	struct Total total23 = {2, &filter2, NULL, NULL};
	struct Total total22 = {4, &filter1, NULL, &total23};
	struct Total total21 = {8, &filter3, NULL, &total22};
	
	struct Adapter adapter3 = {NULL, NULL, &total31, NULL};
	struct Adapter adapter2 = {NULL, NULL, &total21, &adapter3};
	struct Adapter adapter1 = {NULL, NULL, NULL,     &adapter2};
	
	assert_int_equal(0, getTotalForFilter(&adapter1, 0));
	assert_int_equal(5, getTotalForFilter(&adapter1, 1));
	assert_int_equal(2, getTotalForFilter(&adapter1, 2));
	assert_int_equal(8, getTotalForFilter(&adapter1, 3));
}

void testGetMaxFilterDescWidth(void **state){
	struct Filter filter4 = {4, "123", NULL, NULL, NULL, NULL};
	struct Filter filter3 = {3, NULL,  NULL, NULL, NULL, &filter4};
	struct Filter filter2 = {2, "1",   NULL, NULL, NULL, &filter3};
	struct Filter filter1 = {1, "12",  NULL, NULL, NULL, &filter2};

	assert_int_equal(3, getMaxFilterDescWidth(&filter1));
}

void testGetMaxFilterNameWidth(void **state){
	struct Filter filter4 = {4, NULL, "123", NULL, NULL, NULL};
	struct Filter filter3 = {3, NULL, NULL,  NULL, NULL, &filter4};
	struct Filter filter2 = {2, NULL, "1",   NULL, NULL, &filter3};
	struct Filter filter1 = {1, NULL, "12",  NULL, NULL, &filter2};

	assert_int_equal(3, getMaxFilterNameWidth(&filter1));
}
