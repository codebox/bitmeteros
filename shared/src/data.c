#define _GNU_SOURCE
#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

/*
Contains functions for creating and updating Data struts.
*/

struct Data* allocData(){
 // Create a Data struct on the heap
	struct Data* data = (struct Data*) malloc( sizeof( struct Data ) );

	data->ts = 0;
	data->vl = 0;
	data->fl = 0;
	data->dr = 0;
	data->next = NULL;

	return data;
}

struct Data makeData(){
 // Create a Data struct on the stack
	struct Data data;

	data.ts = 0;
	data.vl = 0;
	data.fl = 0;
	data.dr = 0;
	data.next = NULL;

	return data;
}

void freeData(struct Data* data){
 // Free up the memory used by this Data struct, and all others that can be reached through the 'next' pointer chain
	struct Data* next;
	while(data != NULL){
		next = data->next;
		free(data);
		data = next;
	}
}

void appendData(struct Data** earlierData, struct Data* newData){
 /* Add the 'newData' argument to the end of the Data struct list that begins with
    'earlierData - this involves stepping through the list one struct at a time, not
    vey efficient for long lists but ok for all the current usage scenarios. */
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
