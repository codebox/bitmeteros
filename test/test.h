/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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

#include "CuTest.h"
#include "common.h"
#include <time.h>
#include <stdio.h>

#define _BSD_SOURCE 1

void setup();
void setupDbForTest();
void emptyDb();
void addDbRow(time_t ts, int dr, char* ad, int dl, int ul, char* hs);
void addDbRowBinaryAddress(time_t ts, int dr, int dl, int ul, char* ad, int adLen);
void addConfigRow(char* key, char* value);
void setUpDbForTest();
void populateConfigTable();
time_t makeTs(const char* dateTxt);
time_t makeTsUtc(const char* dateTxt);
void setTime(time_t newTime);
int makeTmpFile();
FILE* makeTmpFileStream();
char* readTmpFile();
void freeTmpFile();
void checkData(CuTest *tc, struct Data* data, time_t ts, int dr, char* ad, int dl, int ul, char* hs);
void parseCommandLine(char* cmdLineTxt, char*** argv, int* argc);
void checkDateCriteriaPart(CuTest *tc, struct DateCriteriaPart* part, int isRelative, int val1, int val2, int next);

CuSuite* sqlGetSuite();
CuSuite* processGetSuite();
CuSuite* commonGetSuite();
CuSuite* timeGetSuite();
CuSuite* dataGetSuite();
CuSuite* clientSummaryGetSuite();
CuSuite* clientMonitorGetSuite();
CuSuite* clientDumpGetSuite();
CuSuite* clientQueryGetSuite();
CuSuite* httpRequestGetSuite();
CuSuite* handleConfigGetSuite();
CuSuite* handleMonitorGetSuite();
CuSuite* handleQueryGetSuite();
CuSuite* handleSummaryGetSuite();
CuSuite* handleRssGetSuite();
CuSuite* optionsGetSuite();
CuSuite* bmdbOptionsGetSuite();
CuSuite* bmdbConfigGetSuite();
CuSuite* bmdbUpgradeGetSuite();
CuSuite* clientSyncSuite();
CuSuite* handleSyncGetSuite();
CuSuite* syncOptionsGetSuite();
CuSuite* dbGetSuite();
CuSuite* alertGetSuite();
CuSuite* clientAlertSuite();
CuSuite* clientUtilSuite();
CuSuite* handleFileGetSuite();
CuSuite* handleAlertGetSuite();
