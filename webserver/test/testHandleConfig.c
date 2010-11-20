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

#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include "CuTest.h"
#include "bmws.h"

static void testConfigUpdateOk(CuTest *tc, char* name, char* value);
static void testConfigUpdateOkChanged(CuTest *tc, char* name, char* valueIn, char* valueOut);
static void testConfigUpdateErr(CuTest *tc, char* name, char* value);

/*
Contains unit tests for the handleConfig module.
*/

void testConfigWithAdmin(CuTest *tc) {
    emptyDb();

    addDbRow(100, 1, "eth0", 1, 1, "");
    addDbRow(101, 1, "eth0", 1, 1, "");
    addDbRow(102, 1, "eth1", 1, 1, "");
    addDbRow(103, 1, "eth0", 1, 1, "host1");
    addDbRow(103, 1, "eth0", 1, 1, "host2");
    addDbRow(103, 1, "eth1", 1, 1, "host1");
    addDbRow(103, 1, "eth0", 1, 1, "host2");

    addConfigRow(CONFIG_WEB_MONITOR_INTERVAL, "1");
    addConfigRow(CONFIG_WEB_SUMMARY_INTERVAL, "2");
    addConfigRow(CONFIG_WEB_HISTORY_INTERVAL, "3");
    addConfigRow(CONFIG_WEB_SERVER_NAME,      "server");
    addConfigRow(CONFIG_WEB_COLOUR_DL,        "#ff0000");
    addConfigRow(CONFIG_WEB_COLOUR_UL,        "#00ff00");
    addConfigRow(CONFIG_WEB_RSS_HOST,         "rsshost");
    addConfigRow(CONFIG_WEB_RSS_FREQ,         "1");
    addConfigRow(CONFIG_WEB_RSS_ITEMS,        "10");

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    struct Request req = {"GET", "/config", NULL, NULL};
    processConfigRequest(tmpFd, &req, TRUE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/x-javascript" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "var config = { \"monitorInterval\" : 1, \"summaryInterval\" : 2, \"historyInterval\" : 3"
        ", \"monitorIntervalMin\" : 1000, \"monitorIntervalMax\" : 30000, \"historyIntervalMin\" : 5000"
        ", \"historyIntervalMax\" : 60000, \"summaryIntervalMin\" : 1000, \"summaryIntervalMax\" : 60000"
        ", \"serverName\" : \"server\", \"dlColour\" : \"#ff0000\", \"ulColour\" : \"#00ff00\""
        ", \"allowAdmin\" : 1, \"version\" : \"" VERSION
        "\", \"rssItems\" : 10, \"rssFreq\" : 1, \"rssHost\" : \"rsshost\", "
        "\"adapters\" : [{\"hs\" : \"local\",\"ad\" : \"eth0\"},{\"hs\" : \"local\",\"ad\" : \"eth1\"},{\"hs\" : \"host1\",\"ad\" : \"eth0\"},{\"hs\" : \"host1\",\"ad\" : \"eth1\"},{\"hs\" : \"host2\",\"ad\" : \"eth0\"}] };"
    , result);
}

void testConfigWithoutAdmin(CuTest *tc) {
    emptyDb();

    addDbRow(100, 1, "eth0", 1, 1, "");
    addDbRow(101, 1, "eth0", 1, 1, "");
    addDbRow(102, 1, "eth1", 1, 1, "");
    addDbRow(103, 1, "eth0", 1, 1, "host1");
    addDbRow(103, 1, "eth0", 1, 1, "host2");
    addDbRow(103, 1, "eth1", 1, 1, "host1");
    addDbRow(103, 1, "eth0", 1, 1, "host2");

    addConfigRow(CONFIG_WEB_MONITOR_INTERVAL, "1");
    addConfigRow(CONFIG_WEB_SUMMARY_INTERVAL, "2");
    addConfigRow(CONFIG_WEB_HISTORY_INTERVAL, "3");
    addConfigRow(CONFIG_WEB_SERVER_NAME,      "server");
    addConfigRow(CONFIG_WEB_COLOUR_DL,        "#ff0000");
    addConfigRow(CONFIG_WEB_COLOUR_UL,        "#00ff00");
    addConfigRow(CONFIG_WEB_RSS_HOST,         "rsshost");
    addConfigRow(CONFIG_WEB_RSS_FREQ,         "1");
    addConfigRow(CONFIG_WEB_RSS_ITEMS,        "10");

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    struct Request req = {"GET", "/config", NULL, NULL};
    processConfigRequest(tmpFd, &req, FALSE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/x-javascript" HTTP_EOL
        "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
        "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
        "Connection: Close" HTTP_EOL HTTP_EOL
        "var config = { \"monitorInterval\" : 1, \"summaryInterval\" : 2, \"historyInterval\" : 3"
        ", \"monitorIntervalMin\" : 1000, \"monitorIntervalMax\" : 30000, \"historyIntervalMin\" : 5000"
        ", \"historyIntervalMax\" : 60000, \"summaryIntervalMin\" : 1000, \"summaryIntervalMax\" : 60000"
        ", \"serverName\" : \"server\", \"dlColour\" : \"#ff0000\", \"ulColour\" : \"#00ff00\", \"allowAdmin\" : 0, \"version\" : \"" VERSION
        "\", \"rssItems\" : 10, \"rssFreq\" : 1, \"rssHost\" : \"rsshost\", "
        "\"adapters\" : [{\"hs\" : \"local\",\"ad\" : \"eth0\"},{\"hs\" : \"local\",\"ad\" : \"eth1\"},{\"hs\" : \"host1\",\"ad\" : \"eth0\"},{\"hs\" : \"host1\",\"ad\" : \"eth1\"},{\"hs\" : \"host2\",\"ad\" : \"eth0\"}] };"
    , result);
}

void testConfigUpdateWithoutAdmin(CuTest *tc) {
    emptyDb();

    addConfigRow(CONFIG_WEB_RSS_HOST, "rsshost");

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    struct NameValuePair param = {"anyparam", "anyvalue", NULL};
    struct Request req = {"GET", "/config", &param, NULL};
    processConfigRequest(tmpFd, &req, FALSE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 403 Forbidden" HTTP_EOL
		"Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
		"Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
		"Connection: Close" HTTP_EOL HTTP_EOL
    , result);
}

void testConfigUpdateServerName(CuTest *tc) {
	testConfigUpdateOk(tc,  "web.server_name", "newname");
	testConfigUpdateOk(tc,  "web.server_name", "12345678901234567890123456789012");
	testConfigUpdateErr(tc, "web.server_name", "123456789012345678901234567890123");
	testConfigUpdateErr(tc, "web.server_name", "<script></script>");
}

void testConfigUpdateMonitorInterval(CuTest *tc) {
	testConfigUpdateErr(tc, "web.monitor_interval", "999");
	testConfigUpdateOk(tc,  "web.monitor_interval", "1000");
	testConfigUpdateOk(tc,  "web.monitor_interval", "30000");
	testConfigUpdateErr(tc, "web.monitor_interval", "30001");
	testConfigUpdateErr(tc, "web.monitor_interval", "x");
}

void testConfigUpdateHistoryInterval(CuTest *tc) {
	testConfigUpdateErr(tc, "web.history_interval", "4999");
	testConfigUpdateOk(tc,  "web.history_interval", "5000");
	testConfigUpdateOk(tc,  "web.history_interval", "60000");
	testConfigUpdateErr(tc, "web.history_interval", "60001");
	testConfigUpdateErr(tc, "web.history_interval", "x");
}

void testConfigUpdateSummaryInterval(CuTest *tc) {
	testConfigUpdateErr(tc, "web.summary_interval", "999");
	testConfigUpdateOk(tc,  "web.summary_interval", "1000");
	testConfigUpdateOk(tc,  "web.summary_interval", "60000");
	testConfigUpdateErr(tc, "web.summary_interval", "60001");
	testConfigUpdateErr(tc, "web.summary_interval", "x");
}

void testConfigUpdateRssFreq(CuTest *tc) {
	testConfigUpdateErr(tc, "web.rss.freq", "0");
	testConfigUpdateOk(tc,  "web.rss.freq", "1");
	testConfigUpdateOk(tc,  "web.rss.freq", "2");
	testConfigUpdateErr(tc, "web.rss.freq", "3");
	testConfigUpdateErr(tc, "web.rss.freq", "x");
}

void testConfigUpdateRssItems(CuTest *tc) {
	testConfigUpdateErr(tc, "web.rss.items", "0");
	testConfigUpdateOk(tc,  "web.rss.items", "1");
	testConfigUpdateOk(tc,  "web.rss.items", "20");
	testConfigUpdateErr(tc, "web.rss.items", "21");
	testConfigUpdateErr(tc, "web.rss.items", "x");
}

void testConfigUpdateDlColour(CuTest *tc) {
	testConfigUpdateOkChanged(tc, "web.colour_dl", "012345", "#012345");
	testConfigUpdateOkChanged(tc, "web.colour_dl", "abcdef", "#abcdef");
	testConfigUpdateErr(tc, "web.colour_dl", "00000g");
	testConfigUpdateErr(tc, "web.colour_dl", "12345");
	testConfigUpdateErr(tc, "web.colour_dl", "#123456");
}

void testConfigUpdateUlColour(CuTest *tc) {
	testConfigUpdateOkChanged(tc, "web.colour_ul", "012345", "#012345");
	testConfigUpdateOkChanged(tc, "web.colour_ul", "abcdef", "#abcdef");
	testConfigUpdateErr(tc, "web.colour_ul", "00000g");
	testConfigUpdateErr(tc, "web.colour_ul", "12345");
	testConfigUpdateErr(tc, "web.colour_ul", "#123456");
}

void testConfigUpdateDisallowedParam(CuTest *tc) {
	testConfigUpdateErr(tc, "db.version", "1");
}

static void testConfigUpdateOk(CuTest *tc, char* name, char* value) {
	testConfigUpdateOkChanged(tc, name, value, value);
}
static void testConfigUpdateOkChanged(CuTest *tc, char* name, char* valueIn, char* valueOut) {
    emptyDb();

    addConfigRow(name, "");

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    struct NameValuePair param = {name, valueIn, NULL};
    struct Request req = {"GET", "/config", &param, NULL};
    processConfigRequest(tmpFd, &req, TRUE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 200 OK" HTTP_EOL
        "Content-Type: application/json" HTTP_EOL
		"Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
		"Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
		"Connection: Close" HTTP_EOL HTTP_EOL
		"{}"
    , result);

    CuAssertStrEquals(tc, valueOut, getConfigText(name, FALSE));
}

static void testConfigUpdateErr(CuTest *tc, char* name, char* value) {
    emptyDb();

    addConfigRow(name, "");

    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);

    int tmpFd = makeTmpFile();
    struct NameValuePair param = {name, value, NULL};
    struct Request req = {"GET", "/config", &param, NULL};
    processConfigRequest(tmpFd, &req, TRUE);

    char* result = readTmpFile();

    CuAssertStrEquals(tc,
        "HTTP/1.0 500 Bad/missing parameter" HTTP_EOL
		"Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
		"Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
		"Connection: Close" HTTP_EOL HTTP_EOL
    , result);

    CuAssertStrEquals(tc, "", getConfigText(name, FALSE));
}

CuSuite* handleConfigGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testConfigWithAdmin);
    SUITE_ADD_TEST(suite, testConfigWithoutAdmin);
    SUITE_ADD_TEST(suite, testConfigUpdateWithoutAdmin);
    SUITE_ADD_TEST(suite, testConfigUpdateDisallowedParam);
    SUITE_ADD_TEST(suite, testConfigUpdateServerName);
    SUITE_ADD_TEST(suite, testConfigUpdateMonitorInterval);
    SUITE_ADD_TEST(suite, testConfigUpdateHistoryInterval);
    SUITE_ADD_TEST(suite, testConfigUpdateSummaryInterval);
    SUITE_ADD_TEST(suite, testConfigUpdateRssFreq);
    SUITE_ADD_TEST(suite, testConfigUpdateRssItems);
    SUITE_ADD_TEST(suite, testConfigUpdateDlColour);
    SUITE_ADD_TEST(suite, testConfigUpdateUlColour);
    return suite;
}
