#include <string.h>
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <stdlib.h>
#include <cmockery.h> 
#include "test.h"
#include "common.h"

/*
Contains unit tests for the 'data' module.
*/
void testAllocData(void** state){
    struct Data* data = allocData();
    
    assert_int_equal(0, data->ts);
    assert_int_equal(0, data->dr);
    assert_int_equal(0, data->vl);
    assert_int_equal(0, data->fl);
    assert_true(data->next == NULL);
    
    freeData(data);
}

void testMakeData(void** state){
    struct Data data = makeData();
    
    assert_int_equal(0, data.ts);
    assert_int_equal(0, data.dr);
    assert_int_equal(0, data.vl);
    assert_int_equal(0, data.fl);
    assert_true(data.next == NULL);
}

void testFreeData(void** state){
    struct Data* data1 = allocData();
    struct Data* data2 = allocData();
    struct Data* data3 = allocData();

    data1->next = data2;
    data2->next = data3;
    
    freeData(data1);
}

void testAppendData(void** state){
    struct Data* data0 = NULL;
    
 // Check it works with a null initial element
    struct Data* data1 = allocData();
    appendData(&data0, data1);
    assert_int_equal(data1, data0);
    assert_true(data1->next == NULL);
    
 // Check we append correctly when initial element is non-null
    struct Data* data2 = allocData();
    appendData(&data0, data2);
    assert_int_equal(data1, data0);
    assert_int_equal(data2, data1->next);
    assert_true(data2->next == NULL);
    
 // Check we append correctly when using an intermediate list item as the first argument
    struct Data* data3 = allocData();
    appendData(&data1, data3);
    assert_int_equal(data1, data0);
    assert_int_equal(data2, data1->next);
    assert_int_equal(data3, data2->next);
    assert_true(data3->next == NULL);
    
    freeData(data0);
}
