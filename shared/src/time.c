/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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

#include <stdlib.h>
#include <time.h>
#include "common.h"

/*
Contains code for performing date-based calculations.
*/

#ifdef _WIN32
	time_t timegm(struct tm* t1TmGmt){
		time_t t2LocalSecs = mktime(t1TmGmt);
		struct tm* t2TmGmt = gmtime(&t2LocalSecs);
		time_t t3LocalSecs = mktime(t2TmGmt);
		return t2LocalSecs - (t3LocalSecs - t2LocalSecs);
	}
#endif

time_t getCurrentLocalYearForTs(time_t ts){
 // Returns a timestamp value representing the start of the year in which 'ts' occurs
	struct tm *t = localtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;

	return mktime(t);
}

time_t getCurrentLocalMonthForTs(time_t ts){
 // Returns a timestamp value representing the start of the month in which 'ts' occurs
	struct tm *t = localtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;

	return mktime(t);
}

time_t getCurrentLocalDayForTs(time_t ts){
 // Returns a timestamp value representing the start of the day in which 'ts' occurs
	struct tm *t = localtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;

	return mktime(t);
}

time_t getNextYearForTs(time_t ts){
 // Returns a timestamp value representing the start of the year following the one in which 'ts' occurs, calculated for the GMT timezone
	struct tm *t = gmtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;
	t->tm_year += 1;

	return timegm(t);
}

time_t getNextLocalYearForTs(time_t ts){
 // Returns a timestamp value representing the start of the year following the one in which 'ts' occurs, calculated for the local timezone
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
 // Returns a timestamp value representing the start of the month following the one in which 'ts' occurs, calculated for the GMT timezone
	struct tm *t = gmtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  += 1;

	return timegm(t);
}

time_t getNextLocalMonthForTs(time_t ts){
 // Returns a timestamp value representing the start of the month following the one in which 'ts' occurs, calculated for the local timezone
	struct tm *t = localtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  += 1;

	return mktime(t);
}
time_t getNextDayForTs(time_t ts){
 // Returns a timestamp value representing the start of the day following the one in which 'ts' occurs, calculated for the GMT timezone
	struct tm *t = gmtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday += 1;

	return timegm(t);
}

time_t getNextLocalDayForTs(time_t ts){
 // Returns a timestamp value representing the start of the day following the one in which 'ts' occurs, calculated for the local timezone
	struct tm *t = localtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday += 1;

	return mktime(t);
}

time_t getNextHourForTs(time_t ts){
 // Returns a timestamp value representing the start of the hour following the one in which 'ts' occurs
	int modValue = (int)ts % 3600;
	return (time_t)(ts + 3600 - modValue);
}

time_t getNextMinForTs(time_t ts){
 // Returns a timestamp value representing the start of the minute following the one in which 'ts' occurs
	int modValue = (int)ts % 60;
	return (time_t)(ts + 60 - modValue);
}

time_t addToDate(time_t ts, char unit, int num){
 // Returns a timestamp value obtained by adding the specified number of hours/days/months/years to the 'ts' argument
	if (unit == 'h'){
		return ts + (3600 * num);

	} else {
		struct tm *t = localtime(&ts);

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

struct tm getLocalTime(time_t t){
 // Returns a safe copy of the tm struct returned by localtime
	return *(localtime(&t));
}

void normaliseTm(struct tm* t){
 // Normalise the tm struct, ie adjust any values that have overflowed their normal ranges
    time_t ts = mktime(t);
    struct tm* tm = localtime(&ts);
    t->tm_year = tm->tm_year;
    t->tm_mon  = tm->tm_mon;
    t->tm_mday = tm->tm_mday;
    t->tm_hour = tm->tm_hour;
}
