#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#include <stdlib.h>
#include <time.h>
#include "common.h"

/*
Contains code for performing date-based calculations.
*/

#ifndef UNIT_TESTING
	time_t getTime(){
		return time(NULL);
	}
#else
	static time_t now;
	void setTime(time_t newTime){
	    now = newTime;
	}
	time_t getTime(){
	    return now;
	}
#endif

time_t _timegm(struct tm* t1TmGmt){
	time_t t2LocalSecs = mktime(t1TmGmt);
	struct tm* t2TmGmt = gmtime(&t2LocalSecs);
	time_t t3LocalSecs = mktime(t2TmGmt);
	return t2LocalSecs - (t3LocalSecs - t2LocalSecs);
}
#ifdef _WIN32
	time_t timegm(struct tm* t1TmGmt){
		return _timegm(t1TmGmt);
	}
#endif

time_t getCurrentYearForTs(time_t ts){
 // Returns a timestamp value representing the start of the year in which 'ts' occurs
	struct tm *t = gmtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;

	return timegm(t);
}

time_t getCurrentMonthForTs(time_t ts){
 // Returns a timestamp value representing the start of the month in which 'ts' occurs
	struct tm *t = gmtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;

	return timegm(t);
}

time_t getCurrentDayForTs(time_t ts){
 // Returns a timestamp value representing the start of the day in which 'ts' occurs
	struct tm *t = gmtime(&ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;

	return timegm(t);
}

time_t getNextYearForTs(time_t ts){
 // Returns a timestamp value representing the start of the year following the one in which 'ts' occurs
	struct tm *t = gmtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;
	t->tm_year += 1;

	return timegm(t);
}

time_t getNextMonthForTs(time_t ts){
 // Returns a timestamp value representing the start of the month following the one in which 'ts' occurs
	struct tm *t = gmtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  += 1;

	return timegm(t);
}

time_t getNextDayForTs(time_t ts){
 // Returns a timestamp value representing the start of the day following the one in which 'ts' occurs
	struct tm *t = gmtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday += 1;

	return timegm(t);
}

time_t getNextHourForTs(time_t ts){
 // Returns a timestamp value representing the start of the hour following the one in which 'ts' occurs
	struct tm *t = gmtime(&ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour += 1;

	return timegm(t);
}

time_t getNextMinForTs(time_t ts){
 // Returns a timestamp value representing the start of the minute following the one in which 'ts' occurs
	struct tm *t = gmtime(&ts);

	t->tm_sec = 0;
	t->tm_min += 1;

	return timegm(t);
}

time_t addToDate(time_t ts, char unit, int num){
 // Returns a timestamp value obtained by adding the specified number of hours/days/months/years to the 'ts' argument
	if (unit == 'h'){
		return ts + (3600 * num);

	} else {
		struct tm *t = gmtime(&ts);

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

		return timegm(t);
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
