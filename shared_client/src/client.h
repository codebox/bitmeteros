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

#ifndef CLIENT_H
#define CLIENT_H

#include <time.h>
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
struct Summary{
	struct Data* today;
	struct Data* month;
	struct Data* year;
	struct Data* total;
	time_t tsMin;
	time_t tsMax;
};
// ----
struct ValueBounds{
	unsigned long long min;
	unsigned long long max;
};

struct Data* getMonitorValues(int ts);
struct Summary getSummaryValues();
struct ValueBounds* calcTsBounds();
struct Data* calcMaxValues();
struct Data* getQueryValues();
struct Data* getSyncValues(int ts);
void getDumpValues(void (*callback)(struct Data*));

#endif
