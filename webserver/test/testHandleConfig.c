#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"
#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include "bmws.h"

static void testConfigUpdateOk(char* name, char* value);
static void testConfigUpdateOkChanged(char* name, char* valueIn, char* valueOut);
static void testConfigUpdateErr(char* name, char* value);

/*
Contains unit tests for the handleConfig module.
*/
void testConfigWithAdmin(void** state) {
    addFilterRow(1, "Filter 1", "f1", "", "host1");
    addFilterRow(2, "Filter 2", "f2", "", "host2");
    addFilterRow(3, "Filter 3", "f3", "", "host3");
    
    addConfigRow(CONFIG_WEB_MONITOR_INTERVAL, "1");
    addConfigRow(CONFIG_WEB_SUMMARY_INTERVAL, "2");
    addConfigRow(CONFIG_WEB_HISTORY_INTERVAL, "3");
    addConfigRow(CONFIG_WEB_SERVER_NAME,      "server");
    addConfigRow(CONFIG_WEB_RSS_HOST,         "rsshost");
    addConfigRow(CONFIG_WEB_RSS_FREQ,         "1");
    addConfigRow(CONFIG_WEB_RSS_ITEMS,        "10");
    addConfigRow(CONFIG_WEB_COLOUR ".dl",     "#ff0000");
    addConfigRow(CONFIG_WEB_COLOUR ".ul",     "#00ff00");
    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    expect_string(mockWriteHeadersOk, contentType, "application/x-javascript");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);
    expect_string(mockWriteText, txt, "var config = { ");
    expect_string(mockWriteText, txt, "\"monitorInterval\" : 1");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"summaryInterval\" : 2");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"historyInterval\" : 3");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"monitorIntervalMin\" : 1000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"monitorIntervalMax\" : 30000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"historyIntervalMin\" : 5000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"historyIntervalMax\" : 60000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"summaryIntervalMin\" : 1000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"summaryIntervalMax\" : 60000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"serverName\" : \"server\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"allowAdmin\" : 1");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"version\" : \"" VERSION "\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"rssItems\" : 10");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"rssFreq\" : 1");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"rssHost\" : \"rsshost\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"hosts\" : [");
    expect_string(mockWriteText, txt, "\"");
    expect_string(mockWriteText, txt, "host1");
    expect_string(mockWriteText, txt, "\"");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteText, txt, "\"");
    expect_string(mockWriteText, txt, "host2");
    expect_string(mockWriteText, txt, "\"");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteText, txt, "\"");
    expect_string(mockWriteText, txt, "host3");
    expect_string(mockWriteText, txt, "\"");
    expect_string(mockWriteText, txt, "]");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"filters\" : [");
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteText, txt, "\"id\" : 1");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"name\" : \"f1\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"desc\" : \"Filter 1\"");
    expect_string(mockWriteText, txt, "}");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteText, txt, "\"id\" : 2");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"name\" : \"f2\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"desc\" : \"Filter 2\"");
    expect_string(mockWriteText, txt, "}");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteText, txt, "\"id\" : 3");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"name\" : \"f3\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"desc\" : \"Filter 3\"");
    expect_string(mockWriteText, txt, "}");
    expect_string(mockWriteText, txt, "]");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"web.colour.dl\" : \"#ff0000\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"web.colour.ul\" : \"#00ff00\"");
    expect_string(mockWriteText, txt, " };");
    
    struct Request req = {"GET", "/config", NULL, NULL};
    processConfigRequest(0, &req, TRUE);
    freeStmtList();
}

void testConfigWithoutAdmin(void** state) {
    addFilterRow(1, "Filter 1", "f1", "", NULL);
    addFilterRow(2, "Filter 2", "f2", "", NULL);
    
    addConfigRow(CONFIG_WEB_MONITOR_INTERVAL, "1");
    addConfigRow(CONFIG_WEB_SUMMARY_INTERVAL, "2");
    addConfigRow(CONFIG_WEB_HISTORY_INTERVAL, "3");
    addConfigRow(CONFIG_WEB_SERVER_NAME,      "server");
    addConfigRow(CONFIG_WEB_RSS_HOST,         "rsshost");
    addConfigRow(CONFIG_WEB_RSS_FREQ,         "1");
    addConfigRow(CONFIG_WEB_RSS_ITEMS,        "10");
    addConfigRow(CONFIG_WEB_COLOUR ".dl",     "#ff0000");
    addConfigRow(CONFIG_WEB_COLOUR ".ul",     "#00ff00");

    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    expect_string(mockWriteHeadersOk, contentType, "application/x-javascript");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);
    expect_string(mockWriteText, txt, "var config = { ");
    expect_string(mockWriteText, txt, "\"monitorInterval\" : 1");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"summaryInterval\" : 2");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"historyInterval\" : 3");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"monitorIntervalMin\" : 1000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"monitorIntervalMax\" : 30000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"historyIntervalMin\" : 5000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"historyIntervalMax\" : 60000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"summaryIntervalMin\" : 1000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"summaryIntervalMax\" : 60000");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"serverName\" : \"server\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"allowAdmin\" : 0");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"version\" : \"" VERSION "\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"rssItems\" : 10");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"rssFreq\" : 1");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"rssHost\" : \"rsshost\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"hosts\" : [");
    expect_string(mockWriteText, txt, "]");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"filters\" : [");
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteText, txt, "\"id\" : 1");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"name\" : \"f1\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"desc\" : \"Filter 1\"");
    expect_string(mockWriteText, txt, "}");
    expect_string(mockWriteText, txt, ",");
    expect_string(mockWriteText, txt, "{");
    expect_string(mockWriteText, txt, "\"id\" : 2");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"name\" : \"f2\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"desc\" : \"Filter 2\"");
    expect_string(mockWriteText, txt, "}");
    expect_string(mockWriteText, txt, "]");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"web.colour.dl\" : \"#ff0000\"");
    expect_string(mockWriteText, txt, ", ");
    expect_string(mockWriteText, txt, "\"web.colour.ul\" : \"#00ff00\"");
    expect_string(mockWriteText, txt, " };");
    
    struct Request req = {"GET", "/config", NULL, NULL};
    processConfigRequest(0, &req, FALSE);
    freeStmtList();
}

void testConfigUpdateWithoutAdmin(void** state) {
    addConfigRow(CONFIG_WEB_RSS_HOST, "rsshost");
    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    struct NameValuePair param = {"anyparam", "anyvalue", NULL};
    struct Request req = {"GET", "/config", &param, NULL};
    
    expect_string(mockWriteHeadersForbidden, msg, "config update");
    processConfigRequest(0, &req, FALSE);
    freeStmtList();
}

void testConfigUpdateServerName(void** state) {
    testConfigUpdateOk("web.server_name", "newname");
    testConfigUpdateOk("web.server_name", "12345678901234567890123456789012");
    testConfigUpdateErr("web.server_name", "123456789012345678901234567890123");
    testConfigUpdateErr("web.server_name", "<script></script>");
    freeStmtList();
}

void testConfigUpdateMonitorInterval(void** state) {
    testConfigUpdateErr("web.monitor_interval", "999");
    testConfigUpdateOk("web.monitor_interval", "1000");
    testConfigUpdateOk("web.monitor_interval", "30000");
    testConfigUpdateErr("web.monitor_interval", "30001");
    testConfigUpdateErr("web.monitor_interval", "x");
    freeStmtList();
}

void testConfigUpdateHistoryInterval(void** state) {
    testConfigUpdateErr("web.history_interval", "4999");
    testConfigUpdateOk("web.history_interval", "5000");
    testConfigUpdateOk("web.history_interval", "60000");
    testConfigUpdateErr("web.history_interval", "60001");
    testConfigUpdateErr("web.history_interval", "x");
    freeStmtList();
}

void testConfigUpdateSummaryInterval(void** state) {
    testConfigUpdateErr("web.summary_interval", "999");
    testConfigUpdateOk("web.summary_interval", "1000");
    testConfigUpdateOk("web.summary_interval", "60000");
    testConfigUpdateErr("web.summary_interval", "60001");
    testConfigUpdateErr("web.summary_interval", "x");
    freeStmtList();
}

void testConfigUpdateRssFreq(void** state) {
    testConfigUpdateErr("web.rss.freq", "0");
    testConfigUpdateOk("web.rss.freq", "1");
    testConfigUpdateOk("web.rss.freq", "2");
    testConfigUpdateErr("web.rss.freq", "3");
    testConfigUpdateErr("web.rss.freq", "x");
    freeStmtList();
}

void testConfigUpdateRssItems(void** state) {
    testConfigUpdateErr("web.rss.items", "0");
    testConfigUpdateOk("web.rss.items", "1");
    testConfigUpdateOk("web.rss.items", "20");
    testConfigUpdateErr("web.rss.items", "21");
    testConfigUpdateErr("web.rss.items", "x");
    freeStmtList();
}

void testConfigUpdateDisallowedParam(void** state) {
    testConfigUpdateErr("db.version", "1");
    freeStmtList();
}

static void testConfigUpdateOk(char* name, char* value) {
    testConfigUpdateOkChanged(name, value, value);
}

static void testConfigUpdateOkChanged(char* name, char* valueIn, char* valueOut) {
    addConfigRow(name, "");
    
    time_t now = makeTs("2009-11-08 10:00:00");
    setTime(now);
    
    struct NameValuePair param = {name, valueIn, NULL};
    struct Request req = {"GET", "/config", &param, NULL};
    
    expect_string(mockWriteHeadersOk, contentType, "application/json");
    expect_value(mockWriteHeadersOk, endHeaders, TRUE);
    expect_string(mockWriteText, txt, "{}");
    
    processConfigRequest(0, &req, TRUE);
    
    char* val = getConfigText(name, FALSE);
    assert_string_equal(valueOut, val);
    free(val);
}

static void testConfigUpdateErr(char* name, char* value) {
    //emptyDb();
    //
    //addConfigRow(name, "");
    //
    //time_t now = makeTs("2009-11-08 10:00:00");
    //setTime(now);
    //
    //int tmpFd = makeTmpFile();
    //struct NameValuePair param = {name, value, NULL};
    //struct Request req = {"GET", "/config", &param, NULL};
    //processConfigRequest(tmpFd, &req, TRUE);
    //
    //char* result = readTmpFile();
    //
    //CuAssertStrEquals(tc,
    //    "HTTP/1.0 500 Bad/missing parameter" HTTP_EOL
    //  "Server: BitMeterOS " VERSION " Web Server" HTTP_EOL
    //  "Date: Sun, 08 Nov 2009 10:00:00 +0000" HTTP_EOL
    //  "Connection: Close" HTTP_EOL HTTP_EOL
    //, result);
    //
    //CuAssertStrEquals(tc, "", getConfigText(name, FALSE));
}
