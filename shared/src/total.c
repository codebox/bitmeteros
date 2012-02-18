#include "common.h"
#include <pthread.h>

struct Total* allocTotal(struct Filter* filter){
    struct Total* total = malloc(sizeof(struct Total));

    total->count  = 0;
    total->next   = NULL;
    total->filter = NULL;
    total->handle = NULL;

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    total->mutex = mutex;

    appendFilter(&(total->filter), filter);

    return total;
}

void freeTotals(struct Total* total){
    while (total != NULL) {
        struct Total* next = total->next;
        
        if (total->handle != NULL){
            PCAP_CLOSE(total->handle);
        }
        pthread_mutex_destroy(&(total->mutex));
        free(total);
        
        total = next;
    }
}

void appendTotal(struct Total** earlierTotal, struct Total* newTotal){
    if (*earlierTotal == NULL){
        *earlierTotal = newTotal;
    } else {
        struct Total* curr = *earlierTotal;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newTotal;
    }
}
