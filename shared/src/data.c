/*
 * BitMeterOS v0.1.5
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build Date: Sun, 25 Oct 2009 17:18:38 +0000
 */

#include "common.h"
#include <stdlib.h>
#include <string.h>

/*
Contains functions for creating and updating Data struts.
*/

struct Data* allocData(){
 // Create a Data struct on the heap
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
 // Create a Data struct on the stack
	struct Data data;

	data.ts = 0;
	data.dl = 0;
	data.ul = 0;
	data.dr = 0;
	data.ad = NULL;
	data.next = NULL;

	return data;
}

void setAddress(struct Data* data, const char* addr){
 // Copy the specified address string onto the heap, and associate the Data struct with it
	data->ad = malloc(strlen(addr)+1);
	strcpy(data->ad, addr);
}

void freeData(struct Data* data){
 // Free up the memory used by this Data struct, and all others that can be reached through the 'next' pointer chain
	struct Data* next;
	while(data != NULL){
		next = data->next;
		if (data->ad != NULL){
			free(data->ad);
		}
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
