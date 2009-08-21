#include <stdlib.h>
#include <time.h>
#include "common.h"
//#include "CUTest.h"

/* Performs date-based calculations */

int getCurrentYearForTs(int ts){
 // Returns a timestamp value representing the start of the year in which 'ts' occurs
	struct tm *t = localtime((time_t *) &ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;
	return mktime(t);
}

int getCurrentMonthForTs(int ts){
 // Returns a timestamp value representing the start of the month in which 'ts' occurs
	struct tm *t = localtime((time_t *) &ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	return mktime(t);
}

int getCurrentDayForTs(int ts){
 // Returns a timestamp value representing the start of the day in which 'ts' occurs
	struct tm *t = localtime((time_t *) &ts);
	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	return mktime(t);
}

int getNextYearForTs(int ts){
 // Returns a timestamp value representing the start of the year following the one in which 'ts' occurs
	struct tm *t = localtime((time_t *) &ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  = 0;
	t->tm_year += 1;

	return mktime(t);
}

int getNextMonthForTs(int ts){
 // Returns a timestamp value representing the start of the month following the one in which 'ts' occurs
	struct tm *t = localtime((time_t *) &ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday = 1;
	t->tm_mon  += 1;

	return mktime(t);
}

int getNextDayForTs(int ts){
 // Returns a timestamp value representing the start of the day following the one in which 'ts' occurs
	struct tm *t = localtime((time_t *) &ts);

	t->tm_sec  = 0;
	t->tm_min  = 0;
	t->tm_hour = 0;
	t->tm_mday += 1;

	return mktime(t);
}

int getNextHourForTs(int ts){
 // Returns a timestamp value representing the start of the hour following the one in which 'ts' occurs
	ts += SECS_PER_HOUR;
	struct tm *t = localtime((time_t *) &ts);

	if (t->tm_sec == 0 && t->tm_min == 0){
	 // We were exactly on the hour, so return the ts that was passed in
		return ts;
	} else {
		t->tm_sec = 0;
		t->tm_min = 0;
		return mktime(t);
	}
}

int getNextMinForTs(int ts){
 // Returns a timestamp value representing the start of the minute following the one in which 'ts' occurs
	ts += SECS_PER_MIN;
	struct tm *t = localtime((time_t *) &ts);

	if (t->tm_sec == 0 ){
	 // We were exactly on the minute, so return the ts that was passed in
		return ts;
	} else {
		t->tm_sec = 0;
		return mktime(t);
	}
}

int getYearFromTs(int ts){
 // Returns the year in which 'ts' occurs
	struct tm *t = localtime((time_t *) &ts);
	return 1900 + (t->tm_year);
}

int addToDate(int ts, char unit, int num){
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
