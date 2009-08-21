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

void formatAmount(const unsigned long long amount, const int binary, const int abbrev, char* txt){
	const unsigned long kbMin = (binary ? BINARY_KB_MIN : SI_KB_MIN);
	const unsigned long mbMin = (binary ? BINARY_MB_MIN : SI_MB_MIN);
	const unsigned long long gbMin = (binary ? BINARY_GB_MIN : SI_GB_MIN);
	const unsigned long long tbMin = (binary ? BINARY_TB_MIN : SI_TB_MIN);
	const unsigned long long pbMin = (binary ? BINARY_PB_MIN : SI_PB_MIN);
	const unsigned long long ebMin = (binary ? BINARY_EB_MIN : SI_EB_MIN);

	char* unit;
	float divisor;

	if (amount < kbMin) {
		unit    = (abbrev ? B_SHORT : B_LONG);
		divisor = 1;

	} else if (amount < mbMin) {
		unit    = (abbrev ? KB_SHORT : KB_LONG);
		divisor = kbMin;

	} else if (amount < gbMin) {
		unit    = (abbrev ? MB_SHORT : MB_LONG);
		divisor = mbMin;

	} else if (amount < tbMin) {
		unit    = (abbrev ? GB_SHORT : GB_LONG);
		divisor = gbMin;

	} else if (amount < pbMin) {
		unit    = (abbrev ? TB_SHORT : TB_LONG);
		divisor = tbMin;

	} else if (amount < ebMin) {
		unit    = (abbrev ? PB_SHORT : PB_LONG);
		divisor = pbMin;

	} else {
		unit = (abbrev ? EB_SHORT : EB_LONG);
		divisor = ebMin;
	}

	sprintf(txt, "%1.2f %s", amount/divisor, unit);

}

void toTime(char* timeText, int ts){
	struct tm* cal = localtime((time_t*)&ts);

	int h = cal->tm_hour;
	int m = cal->tm_min;
	int s = cal->tm_sec;

	sprintf(timeText, "%02d:%02d:%02d", h, m, s);
}

void toDate(char* dateText, int ts){
	struct tm* cal = localtime((time_t*)&ts);

	int y = 1900 + cal->tm_year;
	int m = 1 + cal->tm_mon;
	int d = cal->tm_mday;

	sprintf(dateText, "%04d-%02d-%02d", y, m, d);
}
