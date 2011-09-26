#ifndef CLIENT_H
#define CLIENT_H

#include <time.h>
#include "common.h"
// ----
#define QUERY_GROUP_HOURS  1
#define QUERY_GROUP_DAYS   2
#define QUERY_GROUP_MONTHS 3
#define QUERY_GROUP_YEARS  4
#define QUERY_GROUP_TOTAL  5
// ----
#define UNITS_BYTES  1
#define UNITS_ABBREV 2
#define UNITS_FULL   3
// ----
#define LOCAL_HOST "local"
// ----
#define ALERT_ID_FAIL -1
// ----
struct Summary{
	struct Data* today;
	struct Data* month;
	struct Data* year;
	struct Data* total;
	time_t tsMin;
	time_t tsMax;
	char** hostNames;
	int    hostCount;
};
// ----
struct ValueBounds{
	BW_INT min;
	BW_INT max;
};
// ----
struct HostAdapter{
    char* host;
    char* adapter;
};

struct Data* getMonitorValues(int ts, int fl);
struct Summary getSummaryValues();
void freeSummary(struct Summary* summary);
struct ValueBounds* calcTsBounds(int fl);
struct Data* calcMaxValue();
struct Data* getQueryValues();
BW_INT getValueForFilterId(struct Data* data, int filterId);
struct Data* getSyncValues(int ts);
void getDumpValues(int, void (*callback)(int, struct Data*));
int addAlert(struct Alert* alert);
int updateAlert(struct Alert* alert);
struct Alert* getAlerts();
struct Data* getTotalsForAlert(struct Alert* alert, time_t now);
int removeAlert(int id);
struct Data* calcTotalsForAllSince(int ts, char* hs);
struct Data* calcTotalsForHsSince(int ts, char* hs);
struct Data* calcTotalsForHsAdSince(int ts, char* hs);
void formatAmountByUnits(const BW_INT vl, char* vlTxt, int units);
#endif
