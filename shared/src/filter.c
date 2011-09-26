#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include "common.h" 

struct Filter* allocFilter(int id, char* desc, char* name, char* expr, char* host){
	struct Filter* filter = malloc(sizeof(struct Filter));

	filter->id = id;
	
	if (desc != NULL){
		filter->desc = strdup(desc);
	} else {
		filter->desc = NULL;
	}
	
	if (name != NULL) {
		filter->name = strdup(name);
	} else {
		filter->name = NULL;
	}
	
	if (expr != NULL){
		filter->expr = strdup(expr);
	} else {
		filter->expr = NULL;
	}
	
	if (host != NULL){
		filter->host = strdup(host);
	} else {
		filter->host = NULL;
	}
	
	filter->next = NULL;
	
	return filter;
}

void freeFilters(struct Filter* filter){
	while (filter != NULL) {
		struct Filter* next = filter->next;
		if (filter->desc != NULL) {
			free(filter->desc);
		}
		if (filter->name != NULL) {
			free(filter->name);
		}
		if (filter->expr != NULL) {
			free(filter->expr);
		}
		if (filter->host != NULL) {
			free(filter->host);
		}
		free(filter);
		filter = next;
	}
}

struct Filter* getFilterFromId(struct Filter* filters, int id) {
	struct Filter* filter = filters;
	
	while (filter != NULL) {
		if (filter->id == id){
			return filter;	
		}
		filter = filter->next;
	}
	
	return NULL;
}

struct Filter* getFilterFromName(struct Filter* filters, char* name) {
	struct Filter* filter = filters;
		
	while (filter != NULL) {
		if (strcmp(filter->name, name) == 0) {
			return filter;	
		}
		filter = filter->next;
	}
	
	return NULL;
}

void appendFilter(struct Filter** earlierFilter, struct Filter* newFilter){
    if (*earlierFilter == NULL){
        *earlierFilter = newFilter;
    } else {
        struct Filter* curr = *earlierFilter;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newFilter;
    }
}

int getTotalForFilter(struct Adapter* adapters, int filterId){
	int sum = 0;
	
	struct Adapter* adapter = adapters;
	while(adapter != NULL){
		struct Total* total = adapter->total;
		while(total != NULL){
			if (filterId == total->filter->id) {
				sum += total->count;
				total->count = 0;
				break;	
			}	
			total = total->next;
		}
		adapter = adapter->next;
	}
	
	return sum;
}

int getMaxFilterDescWidth(struct Filter* filters){
	int maxLen = 0;
	struct Filter* filter = filters;
	while (filter != NULL){
		if (filter->desc != NULL){
			int len = strlen(filter->desc);
			if (len > maxLen){
				maxLen = len;
			}
		}
		filter = filter->next;
	}
	
	return maxLen;
}


int getMaxFilterNameWidth(struct Filter* filters){
	int maxLen = 0;
	struct Filter* filter = filters;
	while (filter != NULL){
		if (filter->name != NULL){
			int len = strlen(filter->name);
			if (len > maxLen){
				maxLen = len;
			}
		}
		filter = filter->next;
	}

	return maxLen;
}