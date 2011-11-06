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

struct Filter* copyFilter(struct Filter* filter){
    return allocFilter(filter->id, filter->desc, filter->name, filter->expr, filter->host);
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
    
    logMsg(LOG_DEBUG, "getFilterFromId() called with id=%d, but this filter does not exist", id);
    return NULL;
}

int filterHasHost(struct Filter* filter, char* host){
    if (filter->host == NULL) {
        return host == NULL;    
    } else if (host == NULL) {
        return FALSE;
    } else {
        return strcmp(filter->host, host) == 0;
    }
}

static int filterHasName(struct Filter* filter, char* name){
    if (filter->name == NULL) {
        return name == NULL;    
    } else if (name == NULL) {
        return FALSE;
    } else {
        return strcmp(filter->name, name) == 0;
    }
}

struct Filter* getFilterFromName(struct Filter* filters, char* name, char* host) {
    struct Filter* filter = filters;
    while (filter != NULL) {
        if (filterHasName(filter, name) && filterHasHost(filter, host)){
            return filter;
        }
        filter = filter->next;
    }
    
    logMsg(LOG_DEBUG, "getFilterFromName() called with name=%s, host=%s but this filter does not exist", name, host);
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

int filterNameIsValid(char* name) {
    int len = strlen(name);
    if (len < 1 || len > FILTER_NAME_MAX_LENGTH){
        return FAIL;   
    } else {
        while(*name){
            int c = *name;
            if (! ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9'))){
                return FAIL;
            }
            name++;    
        }
        return SUCCESS;
    }
}