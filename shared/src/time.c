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

#include <stdlib.h>
#include <time.h>
#include "common.h"

/*
Contains code for performing date-based calculations.
*/

time_t getCurrentYearForTs(time_t ts){
 // Returns a timestamp value representing the start of the year in which 'ts' occurs
	struct tm *t = localtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;
	return mktime(t);
}

time_t getCurrentMonthForTs(time_t ts){
 // Returns a timestamp value representing the start of the month in which 'ts' occurs
	struct tm *t = localtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	return mktime(t);
}

time_t getCurrentDayForTs(time_t ts){
 // Returns a timestamp value representing the start of the day in which 'ts' occurs
	struct tm *t = localtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	return mktime(t);
}

time_t getNextYearForTs(time_t ts){
 // Returns a timestamp value representing the start of the year following the one in which 'ts' occurs
	struct tm *t = localtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;
	t->tm_year += 1;

	return mktime(t);
}

time_t getNextMonthForTs(time_t ts){
 // Returns a timestamp value representing the start of the month following the one in which 'ts' occurs
	struct tm *t = localtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  += 1;

	return mktime(t);
}

time_t getNextDayForTs(time_t ts){
 // Returns a timestamp value representing the start of the day following the one in which 'ts' occurs
	struct tm *t = localtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday += 1;

	return mktime(t);
}

time_t getNextHourForTs(time_t ts){
 // Returns a timestamp value representing the start of the hour following the one in which 'ts' occurs
	ts += SECS_PER_HOUR;
	struct tm *t = localtime(&ts);

	if (t->tm_sec == 0 && t->tm_min == 0){
	 // We were exactly on the hour, so return the ts that was passed in
		return ts;
	} else {
		t->tm_sec = 0;
		t->tm_min = 0;
		return mktime(t);
	}
}

time_t getNextMinForTs(time_t ts){
 // Returns a timestamp value representing the start of the minute following the one in which 'ts' occurs
	ts += SECS_PER_MIN;
	struct tm *t = localtime(&ts);

	if (t->tm_sec == 0 ){
	 // We were exactly on the minute, so return the ts that was passed in
		return ts;
	} else {
		t->tm_sec = 0;
		return mktime(t);
	}
}

time_t getYearFromTs(time_t ts){
 // Returns the year in which 'ts' occurs
	struct tm *t = localtime(&ts);
	return 1900 + (t->tm_year);
}

time_t addToDate(time_t ts, char unit, int num){
 // Returns a timestamp value obtained by adding the specified number of hours/days/months/years to the 'ts' argument
	if (unit == 'h'){
		return ts + (3600 * num);

	} else {
		struct tm *t = localtime((time_t *) &ts);

		switch(unit){
			case 'd':
				t->tm_mday += num;
				break;
			case 'm':
				t->tm_mon += num;
				break;
			case 'y':
				t->tm_year += num;
				break;
			default:
				break;
		}

		return mktime(t);
	}
}
