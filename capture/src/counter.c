#include "capture.h"
#include "pcap.h"

static struct LockableCounterValue* allocValue(){
    struct LockableCounterValue* value = malloc(sizeof(struct LockableCounterValue));
    
    value->count = 0;
    value->ts = 0;
    value->next = NULL;
    
    return value;
}
struct LockableCounter* allocCounter(){
    struct LockableCounter* counter = malloc(sizeof(struct LockableCounter));
    
    counter->values = NULL;
    counter->fl = 0;
    counter->handle = NULL;

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    counter->mutex = mutex;
    
    counter->next = NULL;

    return counter;
}

void addValueToCounter(struct LockableCounter* counter, time_t ts, int v){
    struct LockableCounterValue* value = counter->values;
    
    while (value != NULL) {
        if (value->ts = ts){
            break;
        }
        value = value->next;
    }
    
    if (value == NULL) {
        value = allocValue();
        value->ts = ts;
        appendCounter(&(counter->values), value);
    }
    
    value->count += v;
}

void freeValue(struct LockableCounterValue* value){
    while (value != NULL) {
        struct LockableCounterValue* next = value->next;
        
        free(value);
        
        value = next;
    }
}

void resetValueForCounter(struct LockableCounter* counter) {
    freeValue(counter->values);
    counter->values = NULL;
}


void appendValue(struct LockableCounterValue** earlierValue, struct LockableCounterValue* newValue){
    if (*earlierValue == NULL) {
        *earlierValue = newValue;
    } else {
        struct LockableCounterValue* curr = *earlierValue;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newValue;
    }
}

void freeCounter(struct LockableCounter* counter){
    while (counter != NULL) {
        struct LockableCounter* next = counter->next;
        
        pthread_mutex_destroy(&(counter->mutex));
        free(counter);
        
        counter = next;
    }
}

void appendCounter(struct LockableCounter** earlierCounter, struct LockableCounter* newCounter){
    if (*earlierCounter == NULL) {
        *earlierCounter = newCounter;
    } else {
        struct LockableCounter* curr = *earlierCounter;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newCounter;
    }
}
