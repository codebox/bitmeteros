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

void testCopyFilter(void** state){
    struct Filter original = {1, "desc", "name", "expr", "host"};   
    struct Filter* copy = copyFilter(&original);
    
    assert_false(&original == copy);
    assert_string_equal("desc", copy->desc);
    assert_string_equal("name", copy->name);
    assert_string_equal("expr", copy->expr);
    assert_string_equal("host", copy->host);
    
    freeFilters(copy);
    
    struct Filter empty = {0, NULL, NULL, NULL, NULL};  
    copy = copyFilter(&empty);
    
    assert_false(&empty == copy);
    assert_true(copy->desc == NULL);
    assert_true(copy->name == NULL);
    assert_true(copy->expr == NULL);
    assert_true(copy->host == NULL);
    
    freeFilters(copy);
}

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

void testFilterHasHost(void** state){
    struct Filter filterWithHost = {1, "d", "n", "e", "host"};
    assert_true(filterHasHost(&filterWithHost, "host"));
    assert_false(filterHasHost(&filterWithHost, "not"));
    assert_false(filterHasHost(&filterWithHost, NULL));
    
    struct Filter filterWithoutHost = {1, "d", "n", "e", NULL};
    assert_false(filterHasHost(&filterWithoutHost, "host"));
    assert_true(filterHasHost(&filterWithoutHost, NULL));
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
    struct Filter* filter1 = allocFilter(FILTER_ID, FILTER_DESC, "A", FILTER_EXPR, NULL);
    struct Filter* filter2 = allocFilter(FILTER_ID, FILTER_DESC, "B", FILTER_EXPR, FILTER_HOST);
    struct Filter* filter3 = allocFilter(FILTER_ID, FILTER_DESC, "C", FILTER_EXPR, NULL);

    filter1->next = filter2;
    filter2->next = filter3;
    
    struct Filter* result;
    
    result = getFilterFromName(filter1, "A", NULL);
    assert_int_equal(filter1, result);
    
    result = getFilterFromName(filter1, "A", "host");
    assert_true(result == NULL);
    
    result = getFilterFromName(filter1, "B", FILTER_HOST);
    assert_int_equal(filter2, result);
    
    result = getFilterFromName(filter1, "B", NULL);
    assert_true(result == NULL);
    
    result = getFilterFromName(filter1, "C", NULL);
    assert_int_equal(filter3, result);
    
    result = getFilterFromName(filter1, "X", NULL);
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

void testFilterNameIsValid(void **state){
    assert_false(filterNameIsValid(""));
    assert_true(filterNameIsValid("F"));
    assert_true(filterNameIsValid("f1"));
    assert_false(filterNameIsValid(" a"));
    assert_false(filterNameIsValid("a "));
    assert_false(filterNameIsValid("a filter"));
    assert_false(filterNameIsValid("a-filter"));
    assert_true(filterNameIsValid("abcdefghijklmnop"));
    assert_false(filterNameIsValid("abcdefghijklmnopq"));
}