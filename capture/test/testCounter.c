#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "capture.h"

/*
Contains unit tests for the counter module.
*/

void testAllocCounterValue(void **state) { 
    struct LockableCounterValue* value = allocValue();
    
    assert_int_equal(0, value->count);
    assert_int_equal(0, value->ts);
    assert_true(value->next == NULL);
    
    freeValue(value);
} 

void testAllocCounter(void **state) { 
    struct LockableCounter* counter = allocCounter(); 
    
    assert_true(counter->values == NULL);
    assert_true(counter->handle == NULL);
    assert_int_equal(0, counter->fl);
    assert_true(counter->next == NULL);
    
    freeCounter(counter);
}

void testAddValueToCounter(void **state){
    struct LockableCounter* counter = allocCounter(); 
    struct LockableCounterValue* value;
    
    assert_true(counter->values == NULL);
    
    time_t ts1 = 100;
    time_t ts2 = 200;
    
    addValueToCounter(counter, ts1, 1);
    value = counter->values;
    assert_int_equal(1,   value->count);
    assert_int_equal(ts1, value->ts);
    assert_true(value->next == NULL);
    
    addValueToCounter(counter, ts1, 2);
    value = counter->values;
    assert_int_equal(3,   value->count);
    assert_int_equal(ts1, value->ts);
    assert_true(value->next == NULL);

    addValueToCounter(counter, ts2, 4);
    value = counter->values;
    assert_int_equal(3,   value->count);
    assert_int_equal(ts1, value->ts);
    value = value->next;
    assert_int_equal(4,   value->count);
    assert_int_equal(ts2, value->ts);
    assert_true(value->next == NULL);
        
    addValueToCounter(counter, ts1, 8);
    value = counter->values;
    assert_int_equal(11,  value->count);
    assert_int_equal(ts1, value->ts);
    value = value->next;
    assert_int_equal(4,   value->count);
    assert_int_equal(ts2, value->ts);
    assert_true(value->next == NULL);
        
    freeCounter(counter);
}

void testResetValueForCounter(void** state){
    struct LockableCounter* counter = allocCounter(); 
    struct LockableCounterValue* value;
    
    time_t ts1 = 100;
    time_t ts2 = 200;
    
    addValueToCounter(counter, ts1, 1);
    addValueToCounter(counter, ts2, 2);
    addValueToCounter(counter, ts1, 4);
    
    value = counter->values;
    assert_int_equal(5,  value->count);
    assert_int_equal(ts1, value->ts);
    value = value->next;
    assert_int_equal(2,   value->count);
    assert_int_equal(ts2, value->ts);
    assert_true(value->next == NULL);

    resetValueForCounter(counter);
    assert_true(counter->values == NULL);
    
    addValueToCounter(counter, ts1, 1);
    addValueToCounter(counter, ts2, 2);
    addValueToCounter(counter, ts1, 4);
    
    value = counter->values;
    assert_int_equal(5,  value->count);
    assert_int_equal(ts1, value->ts);
    value = value->next;
    assert_int_equal(2,   value->count);
    assert_int_equal(ts2, value->ts);
    assert_true(value->next == NULL);
    
    freeCounter(counter);
}

void testAppendValue(void** state){
    struct LockableCounterValue* rootValue = NULL;
    struct LockableCounterValue* value1 = allocValue();
    struct LockableCounterValue* value2 = allocValue();
    struct LockableCounterValue* value3 = allocValue();
    
    appendValue(&rootValue, value1);
    assert_true(rootValue == value1);
    
    appendValue(&rootValue, value2);
    assert_true(rootValue == value1);
    assert_true(rootValue->next == value2);
    
    appendValue(&value2, value3);
    assert_true(rootValue == value1);
    assert_true(rootValue->next == value2);
    assert_true(rootValue->next->next == value3);
    
    freeValue(rootValue);
}

void testAppendCounter(void** state){
    struct LockableCounter* rootCounter = NULL;
    struct LockableCounter* counter1 = allocCounter();
    struct LockableCounter* counter2 = allocCounter();
    struct LockableCounter* counter3 = allocCounter();
    
    appendCounter(&rootCounter, counter1);
    assert_true(rootCounter == counter1);
    
    appendCounter(&rootCounter, counter2);
    assert_true(rootCounter == counter1);
    assert_true(rootCounter->next == counter2);
    
    appendCounter(&counter2, counter3);
    assert_true(rootCounter == counter1);
    assert_true(rootCounter->next == counter2);
    assert_true(rootCounter->next->next == counter3);
    
    freeCounter(rootCounter);
}