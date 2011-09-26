#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include "common.h"

static struct TotalCalls calls = {&pcap_close};

struct TotalCalls getCalls(){
	#ifdef UNIT_TESTING	
		return mockTotalCalls;
	#else
		return calls;
	#endif
}

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
			getCalls().pcap_close(total->handle);
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
