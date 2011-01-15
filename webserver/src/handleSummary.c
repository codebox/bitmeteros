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

#include <stdio.h>
#include "bmws.h"
#include "client.h"
#include "common.h"

/*
Handles '/summary' requests received by the web server.
*/

static void writeTotal(SOCKET, char*, BW_INT, BW_INT);

void processSummaryRequest(SOCKET fd, struct Request* req){
    struct NameValuePair* params = req->params;
    char* ha = getValueForName("ha", params, NULL);

    writeHeadersOk(fd, MIME_JSON, TRUE);

 // Set the host/adapter values if appropriate
    char* hs = NULL;
    char* ad = NULL;
    struct HostAdapter* hostAdapter = NULL;

    if (ha != NULL) {
        hostAdapter = getHostAdapter(ha);
        hs = hostAdapter->host;
        ad = hostAdapter->adapter;
    }

	struct Summary summary = getSummaryValues(hs, ad);

    if (ha != NULL) {
        freeHostAdapter(hostAdapter);
    }

	writeText(fd, "{");
	writeTotal(fd, "today", summary.today->dl, summary.today->ul);
	writeText(fd, ", ");
	writeTotal(fd, "month", summary.month->dl, summary.month->ul);
	writeText(fd, ", ");
	writeTotal(fd, "year",  summary.year->dl,  summary.year->ul);
	writeText(fd, ", ");
	writeTotal(fd, "total", summary.total->dl, summary.total->ul);
	writeText(fd, ", ");

	if (summary.hostNames != NULL){
        writeText(fd, "\"hosts\": [");

        int i;
        for(i=0; i<summary.hostCount; i++){
            if (i>0){
                writeText(fd, ", ");
            }
            writeText(fd, "\"");
            writeText(fd, summary.hostNames[i]);
            writeText(fd, "\"");
        }

        writeText(fd, "]");
	} else {
	    writeText(fd, "\"hosts\": null");
	}

	writeText(fd, ", \"since\": ");

	char sinceTs[12];
    unsigned long since = (unsigned long) summary.tsMin;
	sprintf(sinceTs, "%lu", since);
	writeText(fd, sinceTs);
	writeText(fd, "}");

 // Free memory
	freeData(summary.today);
	freeData(summary.month);
	freeData(summary.year);
	freeData(summary.total);
}

static void writeTotal(SOCKET fd, char* totalName, BW_INT dl, BW_INT ul){
 // Helper function, writes out one of the summary totals in the correct JSON format
 	char prefix[32];
 	sprintf(prefix, "\"%s\": ", totalName);
	writeText(fd, prefix);

	struct Data* data = allocData();
	data->dl = dl;
	data->ul = ul;

	writeSingleDataToJson(fd, data);
	freeData(data);
}

static void formatForMobile(BW_INT amt, char* txt){
	formatAmount(amt, TRUE, UNITS_ABBREV, txt);
}
void processMobileSummaryRequest(SOCKET fd, struct Request* req){
	struct Summary summary = getSummaryValues(NULL, NULL);

 // Daily amounts
	char dlDayTxt[32];
	formatForMobile(summary.today->dl, dlDayTxt);

	char ulDayTxt[32];
	formatForMobile(summary.today->ul, ulDayTxt);

	char cmDayTxt[32];
	formatForMobile(summary.today->dl + summary.today->ul, cmDayTxt);

 // Monthly amounts
	char dlMonthTxt[32];
	formatForMobile(summary.month->dl, dlMonthTxt);

	char ulMonthTxt[32];
	formatForMobile(summary.month->ul, ulMonthTxt);

	char cmMonthTxt[32];
	formatForMobile(summary.month->dl + summary.month->ul, cmMonthTxt);

 // Yearly amounts
	char dlYearTxt[32];
	formatForMobile(summary.year->dl, dlYearTxt);

	char ulYearTxt[32];
	formatForMobile(summary.year->ul, ulYearTxt);

	char cmYearTxt[32];
	formatForMobile(summary.year->dl + summary.year->ul, cmYearTxt);

 // Total amounts
	char dlTotalTxt[32];
	formatForMobile(summary.total->dl, dlTotalTxt);

	char ulTotalTxt[32];
	formatForMobile(summary.total->ul, ulTotalTxt);

	char cmTotalTxt[32];
	formatForMobile(summary.total->dl + summary.total->ul, cmTotalTxt);


	struct NameValuePair pair1  = {"dlDay", dlDayTxt,   NULL};
	struct NameValuePair pair2  = {"ulDay", ulDayTxt,   &pair1};
	struct NameValuePair pair3  = {"cmDay", cmDayTxt,   &pair2};
	struct NameValuePair pair4  = {"dlMonth", dlMonthTxt, &pair3};
	struct NameValuePair pair5  = {"ulMonth", ulMonthTxt, &pair4};
	struct NameValuePair pair6  = {"cmMonth", cmMonthTxt, &pair5};
	struct NameValuePair pair7  = {"dlYear", dlYearTxt,  &pair6};
	struct NameValuePair pair8  = {"ulYear", ulYearTxt,  &pair7};
	struct NameValuePair pair9  = {"cmYear", cmYearTxt,  &pair8};
	struct NameValuePair pair10 = {"dlTotal", dlTotalTxt, &pair9};
	struct NameValuePair pair11 = {"ulTotal", ulTotalTxt, &pair10};
	struct NameValuePair pair12 = {"cmTotal", cmTotalTxt, &pair11};

    processFileRequest(fd, req, &pair12);
}
