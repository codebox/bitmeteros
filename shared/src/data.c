#include "common.h"
#include <stdlib.h>
#include <string.h>

struct Data* allocData(){
	struct Data* data = (struct Data*) malloc( sizeof( struct Data ) );

	data->ts = 0;
	data->dl = 0;
	data->ul = 0;
	data->dr = 0;
	data->ad = NULL;
	data->next = NULL;

	return data;
}

struct Data makeData(){
	struct Data data;

	data.ts = 0;
	data.dl = 0;
	data.ul = 0;
	data.dr = 0;
	data.ad = NULL;
	data.next = NULL;

	return data;
}

void setAddress(struct Data* data, char* addr){
	data->ad = malloc(strlen(addr)+1);
	strcpy(data->ad, addr);
}

void freeData(struct Data* data){
	struct Data* next;
	while(data != NULL){
		next = data->next;
		free(data->ad);
		free(data);
		data = next;
	}
}

void appendData(struct Data** earlierData, struct Data* newData){ //TODO slow for big lists
    if (*earlierData == NULL){
        *earlierData = newData;
    } else {
        struct Data* curr = *earlierData;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newData;
    }
}

