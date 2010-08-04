/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#define BAD_NUM -1
#define MAX_PART_LENGTH 256

void freeDateCriteriaPart(struct DateCriteriaPart* criteriaPart);
void freeDateCriteria(struct DateCriteria* criteria);

struct Alert* allocAlert(){
 // Create an Alert struct on the heap
	struct Alert* alert = (struct Alert*) malloc( sizeof( struct Alert ) );

	alert->id        = 0;
    alert->name      = NULL;
    alert->active    = 0;
    alert->bound     = NULL;
    alert->periods   = NULL;
    alert->direction = 0;
    alert->amount    = 0;
    alert->next      = NULL;

	return alert;
}
void setAlertName(struct Alert* alert, const char* name){
 // Free up the current name if there is one
    if (alert->name != NULL){
        free(alert->name);   
    }
	
    if (name == NULL){
        alert->name = NULL;
        
    } else {
     // Remove any leading/trailing whitespace
        char* nameTrimmed = trim(strdupa(name));

     // Copy the specified name string onto the heap, and associate the Alert struct with it
        alert->name = malloc(strlen(nameTrimmed)+1);
        strcpy(alert->name, nameTrimmed);
    }
}
void appendAlert(struct Alert** earlierAlert, struct Alert* newAlert){
    if (*earlierAlert == NULL){
        *earlierAlert = newAlert;
    } else {
        struct Alert* curr = *earlierAlert;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newAlert;
    }
}
void freeAlert(struct Alert* alert){
 // Free up the memory used by this Alert struct, and all others that can be reached through the 'next' pointer chain
	struct Alert* next;
	while(alert != NULL){
		next = alert->next;
		if (alert->name != NULL){
			free(alert->name);
		}
		if (alert->bound != NULL){
			freeDateCriteria(alert->bound);
		}
		if (alert->periods != NULL){
			freeDateCriteria(alert->periods);
		}
		free(alert);
		alert = next;
	}
}

void freeDateCriteria(struct DateCriteria* criteria){
    struct DateCriteria* next;
	while(criteria != NULL){
		next = criteria->next;
		
		freeDateCriteriaPart(criteria->year);
		freeDateCriteriaPart(criteria->month);
		freeDateCriteriaPart(criteria->day);
		freeDateCriteriaPart(criteria->weekday);
		freeDateCriteriaPart(criteria->hour);
		free(criteria);
		
		criteria = next;
	}
}
void freeDateCriteriaPart(struct DateCriteriaPart* criteriaPart){
    struct DateCriteriaPart* next;
	while(criteriaPart != NULL){
		next = criteriaPart->next;
		free(criteriaPart);
		criteriaPart = next;
	}
}

char* dateCriteriaPartToText(struct DateCriteriaPart* part){
    char* txt = malloc(MAX_PART_LENGTH);
    
    if (part == NULL){
        strcpy(txt, "*");
        
    } else {
    	strcpy(txt, "");
        int firstPart = TRUE;
        
        while(part != NULL){
         // Check if we are getting close to the end of the buffer
        	if (strlen(txt) >= (MAX_PART_LENGTH - 32)){
        		logMsg(LOG_ERR, "DateCriteriaPart contents too large for text buffer of %d chars, text so far is %s", MAX_PART_LENGTH, txt);
        		return txt;	
        	}
        	
            if (!firstPart){
                strcat(txt, ",");
            }
            firstPart = FALSE;
            
            if (part->isRelative){
                strcat(txt, "-");
                sprintf(txt + strlen(txt), "%d", part->val1);
    
            } else if (part->val1 == part->val2) {
                sprintf(txt + strlen(txt), "%d", part->val1);
                
            } else {
                sprintf(txt + strlen(txt), "%d", part->val1);
                strcat(txt, "-");
                sprintf(txt + strlen(txt), "%d", part->val2);
            }
            
            part = part->next;
        }
    }
    return txt;
}
    
struct DateCriteriaPart* makeDateCriteriaPart(char* txt){
	if (txt == NULL){
		logMsg(LOG_ERR, "makeDateCriteriaPart argument was NULL");
		return NULL;
		
	} else if (strcmp("*", txt) == 0){
		return NULL;
		
	} else if (strlen(txt) > MAX_PART_LENGTH){
	    logMsg(LOG_ERR, "makeDateCriteriaPart argument length was too big at %d chars: %s", strlen(txt), txt);
	    return NULL;
		
	} else if (txt[0] == '-') {
		struct DateCriteriaPart* result = malloc(sizeof(struct DateCriteriaPart));
		
		int val = strToInt(txt + 1, BAD_NUM);
		if (val != BAD_NUM){
    		result->isRelative = TRUE;
    		result->val1 = val;
    		result->val2 = 0;
    		result->next = NULL;
    		
    		return result;
    	} else {
    	    logMsg(LOG_ERR, "makeDateCriteriaPart argument was invalid relative part: %s", txt);
    	    return NULL;   
    	}
		
	} else {
		struct DateCriteriaPart* firstResult = NULL;
		struct DateCriteriaPart* lastResult = NULL;

        char* txtCopy = strdupa(txt);
		char* ptr = strtok(txtCopy, ",");
		while (ptr != NULL){
			struct DateCriteriaPart* thisPart = malloc(sizeof(struct DateCriteriaPart));
			thisPart->isRelative = FALSE;
			thisPart->next = NULL;                     
			
			char* hyphenPos;
			if ((hyphenPos = strchr(ptr, '-')) > 0){
			    char part1[hyphenPos - ptr + 1];
			    strncpy(part1, ptr, hyphenPos - ptr);
			    part1[hyphenPos - ptr] = 0;
				thisPart->val1 = strToInt(part1, BAD_NUM);
				thisPart->val2 = strToInt(hyphenPos + 1, BAD_NUM);			
			} else {
				thisPart->val2 = thisPart->val1 = strToInt(ptr, BAD_NUM);
			}
			
			if (thisPart->val1 == BAD_NUM || thisPart->val2 == BAD_NUM || thisPart->val1 > thisPart->val2){
			    firstResult = NULL;
			    break;
			}
			
			if (firstResult == NULL) {
				firstResult = thisPart;
			} else {
				lastResult->next = thisPart;
			}
			lastResult = thisPart;
			
			ptr = strtok(NULL, ",");
		}
		return firstResult;
	}
}

struct DateCriteria* makeDateCriteria(char* yearTxt, char* monthTxt, char* dayTxt, char* weekdayTxt, char* hourTxt){
	struct DateCriteria* result = malloc(sizeof(struct DateCriteria));
	result->year    = makeDateCriteriaPart(yearTxt);
	result->month   = makeDateCriteriaPart(monthTxt);
	result->day     = makeDateCriteriaPart(dayTxt);
	result->weekday = makeDateCriteriaPart(weekdayTxt);
	result->hour    = makeDateCriteriaPart(hourTxt);
	result->next    = NULL;
	
	return result;
}

void appendDateCriteria(struct DateCriteria** earlierCriteria, struct DateCriteria* newCriteria){
    if (*earlierCriteria == NULL){
        *earlierCriteria = newCriteria;
    } else {
        struct DateCriteria* curr = *earlierCriteria;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newCriteria;
    }
}
