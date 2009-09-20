#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>
#include "common.h"
#include "bmclient.h"

extern struct Prefs prefs;
static sqlite3_stmt *stmtGetTotals;
static int doQueryForInterval(char*, int, int);
static void doQuery_GroupTotal();
static void doQuery_GroupYears(int, int);
static void doQuery_GroupMonths(int, int);
static void doQuery_GroupDays(int, int);
static void doQuery_GroupHours(int, int);

/*static void printDate(int ts){
	//TODO remove
	char date[11];
	char time[9];

	toDate(date, ts);
	toTime(time, ts);

	printf("%s %s", date, time);
}*/

void doQuery(){
	if (prefs.rangeFrom == 0 && prefs.rangeTo == 0){
		printf("No range has been specified. Use '-r' to specify a range, use '-h' to display help.\n");

	} else {
		char date1[11];
		char time1[9];
		char date2[11];
		char time2[9];

		toDate(date1, prefs.rangeFrom);
		toTime(time1, prefs.rangeFrom);
		toDate(date2, prefs.rangeTo);
		toTime(time2, prefs.rangeTo);

		printf("From: %s %s\n", date1, time1);
		printf("To:   %s %s\n", date2, time2);

		struct ValuesBounds tsBounds = calcTsBounds();
		prepareSql(&stmtGetTotals, "select sum(dl), sum(ul) from data where ts>? and ts<=?");

		unsigned long minTsFrom = (tsBounds.min > prefs.rangeFrom ? tsBounds.min : prefs.rangeFrom) - 3600;
		unsigned long maxTsTo   = (tsBounds.max < prefs.rangeTo   ? tsBounds.max : prefs.rangeTo);

		switch(prefs.group){
			case PREF_GROUP_HOURS:
				doQuery_GroupHours(minTsFrom, maxTsTo);
				break;

			case PREF_GROUP_DAYS:
				doQuery_GroupDays(minTsFrom, maxTsTo);
				break;

			case PREF_GROUP_MONTHS:
				doQuery_GroupMonths(minTsFrom, maxTsTo);
				break;

			case PREF_GROUP_YEARS:
				doQuery_GroupYears(minTsFrom, maxTsTo);
				break;

			case PREF_GROUP_TOTAL:
				doQuery_GroupTotal(minTsFrom, maxTsTo);
				break;

			default:
				//TODO
				break;

		}
	}
}

/*static void printRange(char* prefix, int from, int to){
	char date3[11];
	char time3[9];
	char date4[11];
	char time4[9];

	toDate(date3, from);
	toTime(time3, from);
	toDate(date4, to);
	toTime(time4, to);
	printf("%s From=%d (%s %s) To=%d (%s %s)\n", prefix, from, date3, time3, to, date4, time4);
}*/

static void doQuery_GroupTotal(int minFrom, int maxTo){
	doQueryForInterval("Total:", minFrom, maxTo);
}

static char* makeYearTxt(int ts, char* txt){
	strftime(txt, 5, "%Y", localtime((time_t *) &ts));
	return txt;
}
static void doQuery_GroupYears(int minFrom, int maxTo){
	int from, to;

	from = minFrom;
	to   = getNextYearForTs(minFrom);

	char yearTxt[5];

	while(to <= maxTo){
		doQueryForInterval(makeYearTxt(from, yearTxt), from, to);
		from = to;
		to   = addToDate(to, 'y', 1);
	}
	doQueryForInterval(makeYearTxt(from, yearTxt), from, maxTo);
}

static char* makeMonthTxt(int ts, char* txt){
	strftime(txt, 9, "%b %Y", localtime((time_t *) &ts));
	return txt;
}
static void doQuery_GroupMonths(int minFrom, int maxTo){
	int from, to;

	from = minFrom;
	to   = getNextMonthForTs(minFrom);

	char monthTxt[9];

	while(to <= maxTo){
		doQueryForInterval(makeMonthTxt(from, monthTxt), from, to);
		from = to;
		to   = addToDate(to, 'm', 1);
	}
	doQueryForInterval(makeMonthTxt(from, monthTxt), from, maxTo);
}

static char* makeDayTxt(int ts, char* txt){
	strftime(txt, 12, "%d %b %Y", localtime((time_t *) &ts));
	return txt;
}
static void doQuery_GroupDays(int minFrom, int maxTo){
	int from, to;

	from = minFrom;
	to   = getNextDayForTs(minFrom);

	char dayTxt[12];

	while(to <= maxTo){
		doQueryForInterval(makeDayTxt(from, dayTxt), from, to);
		from = to;
		to   = addToDate(to, 'd', 1);
	}
	doQueryForInterval(makeDayTxt(from, dayTxt), from, maxTo);
}

static char* makeHourTxt(int ts, char* txt){
	strftime(txt, 6, "%H:00", localtime((time_t *) &ts));
	return txt;
}
static void doQuery_GroupHours(int minFrom, int maxTo){
	int from, to;

	from = minFrom;
	to   = getNextHourForTs(minFrom);

	char dayTxt[12];
	char time1Txt[6];
	char time2Txt[6];
	char hourTxt[23];

	while(to <= maxTo){
		sprintf(hourTxt, "%s %s-%s", makeDayTxt(from, dayTxt), makeHourTxt(from, time1Txt), makeHourTxt(to, time2Txt));
		doQueryForInterval(hourTxt, from, to);
		from = to;
		to   = addToDate(to, 'h', 1);
	}
	sprintf(hourTxt, "%s %s-%s", makeDayTxt(from, dayTxt), makeHourTxt(from, time1Txt), makeHourTxt(to, time2Txt));
	doQueryForInterval(hourTxt, from, maxTo);
}

static int doQueryForInterval(char* rangeTxt, int tsFrom, int tsTo){
	sqlite3_bind_int(stmtGetTotals, 1, tsFrom);
	sqlite3_bind_int(stmtGetTotals, 2, tsTo);

	unsigned long long dlTotal, ulTotal;
	int rc = sqlite3_step(stmtGetTotals);
	if (rc == SQLITE_ROW){
		dlTotal = sqlite3_column_int64(stmtGetTotals, 0);
		ulTotal = sqlite3_column_int64(stmtGetTotals, 1);
	} else {
		dlTotal = ulTotal = 0;
	}
	sqlite3_reset(stmtGetTotals);

	char* dlTxt = (char*) calloc(20, sizeof(char));
	char* ulTxt = (char*) calloc(20, sizeof(char));
	formatAmounts(dlTotal, ulTotal, dlTxt, ulTxt, prefs.units);

	printf("%s DL=%s UL=%s\n", rangeTxt, dlTxt, ulTxt);

	return (dlTotal > 0 || ulTotal > 0) ? 1 : 0;
}
