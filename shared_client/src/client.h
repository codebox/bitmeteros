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

struct Data* getMonitorValues(int ts, char* hs, char* ad);
struct Summary getSummaryValues();
void freeSummary(struct Summary* summary);
struct ValueBounds* calcTsBounds(char* hs, char* ad);
struct Data* calcMaxValues();
struct Data* getQueryValues();
struct Data* getSyncValues(int ts);
void getDumpValues(int, void (*callback)(int, struct Data*));
struct HostAdapter* getHostAdapter(char* hostAndAdapterTxt);
void freeHostAdapter(struct HostAdapter *hostAdapter);
int addAlert(struct Alert* alert);
int updateAlert(struct Alert* alert);
struct Alert* getAlerts();
struct Data* getTotalsForAlert(struct Alert* alert, time_t now);
int removeAlert(int id);
struct Data* calcTotalsForAllSince(int ts, char* hs, char* ad);
struct Data* calcTotalsForHsSince(int ts, char* hs, char* ad);
struct Data* calcTotalsForHsAdSince(int ts, char* hs, char* ad);
void formatAmounts(const BW_INT dl, const BW_INT ul, char* dlTxt, char *ulTxt, int units);
#endif
