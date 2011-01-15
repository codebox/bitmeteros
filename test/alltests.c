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

#include "CuTest.h"
#include "test.h"
#include <stdio.h>

void RunAllTests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite   = CuSuiteNew();

    CuSuiteAddSuite(suite, sqlGetSuite());
    CuSuiteAddSuite(suite, processGetSuite());
    CuSuiteAddSuite(suite, commonGetSuite());
    CuSuiteAddSuite(suite, timeGetSuite());
    CuSuiteAddSuite(suite, dbGetSuite());
    CuSuiteAddSuite(suite, dataGetSuite());
    CuSuiteAddSuite(suite, alertGetSuite());
    CuSuiteAddSuite(suite, clientSummaryGetSuite());
    CuSuiteAddSuite(suite, clientMonitorGetSuite());
    CuSuiteAddSuite(suite, clientDumpGetSuite());
    CuSuiteAddSuite(suite, clientQueryGetSuite());
    CuSuiteAddSuite(suite, clientSyncSuite());
    CuSuiteAddSuite(suite, clientAlertSuite());
    CuSuiteAddSuite(suite, clientUtilSuite());
    CuSuiteAddSuite(suite, httpRequestGetSuite());
    CuSuiteAddSuite(suite, handleConfigGetSuite());
    CuSuiteAddSuite(suite, handleFileGetSuite());
    CuSuiteAddSuite(suite, handleMonitorGetSuite());
    CuSuiteAddSuite(suite, handleQueryGetSuite());
    CuSuiteAddSuite(suite, handleSummaryGetSuite());
    CuSuiteAddSuite(suite, handleSyncGetSuite());
    CuSuiteAddSuite(suite, handleAlertGetSuite());
    CuSuiteAddSuite(suite, handleRssGetSuite());
    CuSuiteAddSuite(suite, optionsGetSuite());
    CuSuiteAddSuite(suite, bmdbConfigGetSuite());
    CuSuiteAddSuite(suite, bmdbUpgradeGetSuite());
    CuSuiteAddSuite(suite, syncOptionsGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(void) {
    setup();
    RunAllTests();
    return 0;
}
