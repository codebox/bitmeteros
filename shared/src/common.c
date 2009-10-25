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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
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
 // Populates the 'timeText' string with the time component of the specified timestamp, in the format HH:MM:SS
	struct tm* cal = localtime(&ts);

	int h = cal->tm_hour;
	int m = cal->tm_min;
	int s = cal->tm_sec;

	sprintf(timeText, "%02d:%02d:%02d", h, m, s);
}

void toDate(char* dateText, time_t ts){
 // Populates the 'dateText' string with the time component of the specified timestamp, in the format yyyy-mm-dd
	struct tm* cal = localtime(&ts);

	int y = 1900 + cal->tm_year;
	int m = 1 + cal->tm_mon;
	int d = cal->tm_mday;

	sprintf(dateText, "%04d-%02d-%02d", y, m, d);
}

/*
static char HEX[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
void makeHexString(char* hexString, char* data){ //TODO test
 // Convert the MAC address bytes that we get back from the API into a hex string
	char thisByte;
	int i;
	for(i = 0; i < MAX_ADDR_BYTES; i++){
		thisByte = data[i];
		hexString[i*2]     = HEX[(thisByte >> 4) & 0xF];
		hexString[i*2 + 1] = HEX[thisByte & 0x0F];
	}
	hexString[MAX_ADDR_BYTES * 2] = 0;
}
*/
