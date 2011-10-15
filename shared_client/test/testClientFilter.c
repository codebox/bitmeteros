#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include <string.h>
#include "common.h"
#include "client.h"
#include "test.h"

#define SQL_GET_FILTERS "select * from filter"

void testReadFilters(void** state){
	struct Filter* filters = readFilters();
	assert_int_equal(filters, NULL);
	
	addFilterRow(1, "filter1", "f1", "x1", NULL);
	addFilterRow(2, "filter2", "f2", "x2", NULL);
	addFilterRow(3, "filter3", "f3", "x3", "host3");

	filters = readFilters();
	struct Filter* filter = filters;
	assert_int_equal(1, filter->id);
	filter = filter->next;
	assert_int_equal(2, filter->id);
	filter = filter->next;
	assert_int_equal(3, filter->id);
	assert_int_equal(NULL, filter->next);
	
	freeFilters(filters);
	freeStmtList();	
}

void testGetFilter(void** state){
	addFilterRow(1, "filter1", "f1", "x1", NULL);
	addFilterRow(2, "filter2", "f2", "x2", NULL);
	addFilterRow(3, "filter3", "f3", "x3", "host3");
	
	struct Filter* filter;
	
	filter = getFilter("f3", NULL);
	assert_true(filter == NULL);
	
	filter = getFilter("f3", "host3");
	checkFilter(filter, 3, "filter3", "f3", "x3", "host3");
	freeFilters(filter);
	
	filter = getFilter("f1", "host3");
	assert_true(filter == NULL);
	
	filter = getFilter("f1", NULL);
	checkFilter(filter, 1, "filter1", "f1", "x1", NULL);
	freeFilters(filter);
	freeStmtList();
}

void testFilterExprIsValid(void** state){
	assert_true(filterExprIsValid("port 80"));
	assert_false(filterExprIsValid("bork"));
}

void testAddFilter(void** state){
	struct Filter* filter = allocFilter(0, "filter 1", "f1", "x1", "h1");
	int id = addFilter(filter);
	assert_int_equal(1, id);
	freeFilters(filter);
	
	filter = allocFilter(0, "filter 2", "f2", "x2", NULL);
	id = addFilter(filter);
	assert_int_equal(2, id);
	freeFilters(filter);
	
	struct Filter* filters = readFilters();
	filter = filters;
	checkFilter(filter, 1, "filter 1", "f1", "x1", "h1");
	
	filter = filter->next;
	checkFilter(filter, 2, "filter 2", "f2", "x2", NULL);
	assert_int_equal(NULL, filter->next);
	
	freeFilters(filters);
	freeStmtList();
}

void testRemoveFilter(void** state){
	int status;
	
	status = removeFilter("notthere", NULL);
	assert_int_equal(FAIL, status);
	
	addFilterRow(1, "filter 1", "f1", "x1", NULL);
	assert_int_equal(1, getRowCount(SQL_GET_FILTERS));
	
	status = removeFilter("notthere", "x");
	assert_int_equal(FAIL, status);
	assert_int_equal(1, getRowCount(SQL_GET_FILTERS));
                                   
	status = removeFilter("f1", NULL); 
	assert_int_equal(SUCCESS, status);
	assert_int_equal(0, getRowCount(SQL_GET_FILTERS));
	
	addFilterRow(1, "filter 1", "f1", "x1", NULL);
	addFilterRow(2, "filter 2", "f2", "x2", "host");
	assert_int_equal(2, getRowCount(SQL_GET_FILTERS));	
	
	status = removeFilter("f2", "host"); 
	assert_int_equal(SUCCESS, status);
	assert_int_equal(1, getRowCount(SQL_GET_FILTERS));	
	
	status = removeFilter("f2", "host"); 
	assert_int_equal(FAIL, status);
	assert_int_equal(1, getRowCount(SQL_GET_FILTERS));	
	
	status = removeFilter("f1", NULL); 
	assert_int_equal(SUCCESS, status);
	assert_int_equal(0, getRowCount(SQL_GET_FILTERS));	
	
	freeStmtList();
}

void testReadFiltersForHost(void** state){
	addFilterRow(1, "filter 1", "f1", "x1", NULL);
	addFilterRow(2, "filter 2", "f2", "x2", "host1");
	addFilterRow(3, "filter 3", "f3", "x3", "host2");
	addFilterRow(4, "filter 4", "f4", "x4", "host1");
	
	struct Filter* filter;
	
	filter = readFiltersForHost(NULL);
	checkFilter(filter, 1, "filter 1", "f1", "x1", NULL);
	assert_true(filter->next == NULL);
	freeFilters(filter);
	freeStmtList();
	
	filter = readFiltersForHost("host2");
	checkFilter(filter, 3, "filter 3", "f3", "x3", "host2");
	assert_true(filter->next == NULL);
	freeFilters(filter);
	freeStmtList();

	filter = readFiltersForHost("host1");
	checkFilter(filter, 2, "filter 2", "f2", "x2", "host1");
	checkFilter(filter->next, 4, "filter 4", "f4", "x4", "host1");
	assert_true(filter->next->next == NULL);
	freeFilters(filter);
	freeStmtList();
}