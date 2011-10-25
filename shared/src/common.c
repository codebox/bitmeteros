#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>       
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include "common.h"


#define BINARY_K 1024
#define BINARY_KB_MIN pow(BINARY_K, 1)
#define BINARY_MB_MIN pow(BINARY_K, 2)
#define BINARY_GB_MIN pow(BINARY_K, 3)
#define BINARY_TB_MIN pow(BINARY_K, 4)
#define BINARY_PB_MIN pow(BINARY_K, 5)
#define BINARY_EB_MIN pow(BINARY_K, 6)

#define SI_K 1000
#define SI_KB_MIN pow(SI_K, 1)
#define SI_MB_MIN pow(SI_K, 2)
#define SI_GB_MIN pow(SI_K, 3)
#define SI_TB_MIN pow(SI_K, 4)
#define SI_PB_MIN pow(SI_K, 5)
#define SI_EB_MIN pow(SI_K, 6)

#define B_SHORT  "B "
#define KB_SHORT "kB"
#define MB_SHORT "MB"
#define GB_SHORT "GB"
#define TB_SHORT "TB"
#define PB_SHORT "PB"
#define EB_SHORT "EB"

#define B_LONG   "bytes"
#define KB_LONG  "kilobytes"
#define MB_LONG  "megabytes"
#define GB_LONG  "gigabytes"
#define TB_LONG  "terabytes"
#define PB_LONG  "petabytes"
#define EB_LONG  "exabytes"

void formatAmount(const BW_INT amount, const int binary, const int abbrev, char* txt){
 /* Populates the 'txt' argument with a string representing the specified BW_INT value
    formatted according to the 'binary' and 'abbrev' values.

    If the 'binary' flag is set, then powers of 1024 are used for unit calculations,
    otherwise decimal values (ie powers of 1000) are used.

    If the 'abbrev' flag is set then abbreviated versions of unit names are used
    otherwise the full unit names are used. */

	const BW_INT kbMin = (binary ? BINARY_KB_MIN : SI_KB_MIN);
	const BW_INT mbMin = (binary ? BINARY_MB_MIN : SI_MB_MIN);
	const BW_INT gbMin = (binary ? BINARY_GB_MIN : SI_GB_MIN);
	const BW_INT tbMin = (binary ? BINARY_TB_MIN : SI_TB_MIN);
	const BW_INT pbMin = (binary ? BINARY_PB_MIN : SI_PB_MIN);
	const BW_INT ebMin = (binary ? BINARY_EB_MIN : SI_EB_MIN);

	char* unit;
	float divisor;

	if (amount < kbMin) {
	 // Display value in bytes
		unit    = (abbrev ? B_SHORT : B_LONG);
		divisor = 1;

	} else if (amount < mbMin) {
	 // Display value in kilobytes
		unit    = (abbrev ? KB_SHORT : KB_LONG);
		divisor = kbMin;

	} else if (amount < gbMin) {
	 // Display value in megabytes
		unit    = (abbrev ? MB_SHORT : MB_LONG);
		divisor = mbMin;

	} else if (amount < tbMin) {
	 // Display value in gigabytes
		unit    = (abbrev ? GB_SHORT : GB_LONG);
		divisor = gbMin;

	} else if (amount < pbMin) {
	 // Display value in terabytes
		unit    = (abbrev ? TB_SHORT : TB_LONG);
		divisor = tbMin;

	} else if (amount < ebMin) {
	 // Display value in exabytes
		unit    = (abbrev ? PB_SHORT : PB_LONG);
		divisor = pbMin;

	} else {
	 // Display value in petabytes
		unit = (abbrev ? EB_SHORT : EB_LONG);
		divisor = ebMin;
	}

 // Use 2 decimal places
	sprintf(txt, "%1.2f %s", amount/divisor, unit);

}

void toTime(char* timeText, time_t ts){
 /* Populates the 'timeText' string with the time component of the specified timestamp, when 
 	expressed as local time, in the format HH:MM:SS */
	struct tm* cal = localtime(&ts);

	int h = cal->tm_hour;
	int m = cal->tm_min;
	int s = cal->tm_sec;

	sprintf(timeText, "%02d:%02d:%02d", h, m, s);
}

void toDate(char* dateText, time_t ts){
 /* Populates the 'dateText' string with the time component of the specified timestamp,  when
 	expressed as local time, in the format yyyy-mm-dd */
	struct tm* cal = localtime(&ts);

	int y = 1900 + cal->tm_year;
	int m = 1 + cal->tm_mon;
	int d = cal->tm_mday;

	sprintf(dateText, "%04d-%02d-%02d", y, m, d);
}


static char HEX[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
void makeHexString(char* hexString, const char* data, int dataLen){
 // Convert the MAC address bytes that we get back from the API into a hex string
	char thisByte;
	int i;
	for(i = 0; i < dataLen; i++){
		thisByte = data[i];
		hexString[i*2]     = HEX[(thisByte >> 4) & 0xF];
		hexString[i*2 + 1] = HEX[thisByte & 0x0F];
	}
	hexString[dataLen * 2] = 0;
}

BW_INT strToBwInt(char* txt, BW_INT defaultValue){
    if (txt == NULL){
        return defaultValue;
    } else {
        char *end;
        BW_INT value = strtoull(txt, &end, 10);
        errno = 0;
        if (end == txt || *end != '\0' || errno == ERANGE){
            return defaultValue;
        } else {
            return value;
        }
    }
}

long strToLong(char* txt, long defaultValue){
    if (txt == NULL){
        return defaultValue;
    } else {
        char *end;
        long value = strtol(txt, &end, 10);
        errno = 0;
        if (end == txt || *end != '\0' || errno == ERANGE){
            return defaultValue;
        } else {
            return value;
        }
    }
}

int strToInt(char* txt, int defaultValue){
    return (int) strToLong(txt, defaultValue);
}

char *trim(char *str){
    char *end;

 // Trim leading space
    while(isspace(*str)){
        str++;
    }

 // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)){
        end--;
    }

 // Write new null terminator
    *(end+1) = 0;

    return str;
}

char* replace(char* src, char* target, char* replace){
	if (src == NULL) {
		return NULL;	
	} else if (target == NULL) {
		return strdup(src);
	} else {
		if (replace == NULL){
			replace = "";	
		}
		char* match;
		char* result = strdup(src);
		while ((match = strstr(result, target)) != NULL) {
			int matchPosn = match - result;
			char* tmp = calloc(strlen(result) + strlen(replace) - strlen(target) + 1, 1);
			strncpy(tmp, result, matchPosn);
			strcat(tmp, replace);
			strcat(tmp, result + matchPosn + strlen(target));
			free(result);	
			result = tmp;
		}
		return result;
	}
}

char* strAppend(char* startTxt, ...){
	va_list argp;
	char* txtToAppend;
	char* tmp;
	char* allText = strdup(startTxt);
	va_start(argp, startTxt);

	while((txtToAppend = va_arg(argp, char*)) != NULL){
		tmp = malloc(strlen(allText) + strlen(txtToAppend) + 1);
		strcpy(tmp, allText);
		strcat(tmp, txtToAppend);
		free(allText);
		allText = tmp;
	}
	va_end(argp);
	
	return allText;
}
