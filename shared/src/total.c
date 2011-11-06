#include "common.h"

struct Total* allocTotal(struct Filter* filter){
    struct Total* total = malloc(sizeof(struct Total));

    total->count  = 0;
    total->next   = NULL;
    total->filter = NULL;
    total->handle = NULL;
    appendFilter(&(total->filter), filter);

    return total;
}

void freeTotals(struct Total* total){
    while (total != NULL) {
        struct Total* next = total->next;
        
        if (total->handle != NULL){
            PCAP_CLOSE(total->handle);
        }
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
