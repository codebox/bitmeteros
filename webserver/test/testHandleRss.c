#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "bmws.h"

/*
Contains unit tests for the handleRss module.
*/

void testRssHourlyNoAlerts(void** state) {
	addFilterRow(FILTER,  "Filter 1", "f1", "expr1", NULL);
	addFilterRow(FILTER2, "Filter 2", "f2", "expr1", NULL);
	
    addDbRow(makeTs("2010-01-10 06:30:00"), 1,  1, FILTER); // not included (too early)
    addDbRow(makeTs("2010-01-10 07:30:00"), 1,  2, FILTER);
    addDbRow(makeTs("2010-01-10 07:30:00"), 1,  3, FILTER2);
    addDbRow(makeTs("2010-01-10 08:30:00"), 1,  4, FILTER);
    addDbRow(makeTs("2010-01-10 09:30:00"), 1,  8, FILTER);
    addDbRow(makeTs("2010-01-10 10:00:10"), 1, 16, FILTER); // not included (too late)
    
	addConfigRow("web.rss.host",    "rsshost");
	addConfigRow("web.rss.freq",    "2");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "");
    
    time_t now = makeTs("2010-01-10 10:00:30");
    setTime(now);
    
	struct NameValuePair* firstValue;
    struct NameValuePair* value = firstValue = makeRssRequestValues();
	assert_string_equal("title", value->name);
	assert_string_equal("BitMeter OS Events", value->value);
	
	value = value->next;
	assert_string_equal("link", value->name);
	assert_string_equal("http://rsshost:2605", value->value);
	
	value = value->next;
	assert_string_equal("pubdate", value->name);
	assert_string_equal("Sun, 10 Jan 2010 10:00:00 +0000", value->value);
    
	value = value->next;
	assert_string_equal("items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Sunday, 10 Jan 9:00-10:00</title>"
			"<description>On Sunday 10 January 2010 between 9:00 and 10:00 totals were as follows:&lt;br&gt;Filter 1: 8.00 B &lt;br&gt;Filter 2: 0.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 10:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://rsshost:2605/#hourly.1263114000.1263117600</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 8:00-9:00</title>"
			"<description>On Sunday 10 January 2010 between 8:00 and 9:00 totals were as follows:&lt;br&gt;Filter 1: 4.00 B &lt;br&gt;Filter 2: 0.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 09:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://rsshost:2605/#hourly.1263110400.1263114000</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 7:00-8:00</title>"
			"<description>On Sunday 10 January 2010 between 7:00 and 8:00 totals were as follows:&lt;br&gt;Filter 1: 2.00 B &lt;br&gt;Filter 2: 3.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 08:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://rsshost:2605/#hourly.1263106800.1263110400</guid>"
		"</item>";

	assert_string_equal(itemsXml, value->value);
    
	value = value->next;
	assert_string_equal("ttl", value->name);
	assert_string_equal("60", value->value);
    
	value = value->next;
	assert_string_equal("freq", value->name);
	assert_string_equal("hourly", value->value);
	
	freeNameValuePairs(firstValue);
	freeStmtList();
}

void testRssWithAlertOk(void** state) {            
	addFilterRow(FILTER,  "Filter 1", "f1", "expr1", NULL);	
	addFilterRow(FILTER2, "Filter 2", "f2", "expr2", NULL);	
	
    time_t now = makeTs("2010-01-10 10:00:30");
    setTime(now);
    
    addDbRow(makeTs("2010-01-10 06:30:00"), 1,  1, FILTER); // not included (too early)
    addDbRow(makeTs("2010-01-10 07:30:00"), 1,  2, FILTER);
    addDbRow(makeTs("2010-01-10 08:30:00"), 1,  4, FILTER);
    addDbRow(makeTs("2010-01-10 09:30:00"), 1,  8, FILTER);
    addDbRow(makeTs("2010-01-10 09:30:00"), 1,  8, FILTER2);
    addDbRow(makeTs("2010-01-10 10:00:10"), 1, 16, FILTER); // not included (too late)
    
	struct Alert* alert1 = allocAlert();
    alert1->id     = 1;
    alert1->active = 1;
    alert1->filter = FILTER;
    alert1->amount = 100;
    alert1->bound  = makeDateCriteria("2010", "1", "1", "5", "0");
    setAlertName(alert1, "alert1");
    
    addAlert(alert1);
	freeAlert(alert1);
	
	struct Alert* alert2 = allocAlert();
    alert2->id     = 2;
    alert2->active = 1;
    alert2->filter = FILTER2;
    alert2->amount = 100;
    alert2->bound  = makeDateCriteria("2010", "1", "1", "5", "0");
    setAlertName(alert2, "alert2");
    
    addAlert(alert2);
	freeAlert(alert2);

	addConfigRow("web.rss.host",    "");
	addConfigRow("web.rss.freq",    "2");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "");
    
    struct NameValuePair* firstValue;
    struct NameValuePair* value = firstValue = makeRssRequestValues();
	assert_string_equal("title", value->name);
	assert_string_equal("BitMeter OS Events", value->value);
    
	value = value->next;
	assert_string_equal("link", value->name);
	assert_string_equal("http://localhost:2605", value->value);
	
	value = value->next;
	assert_string_equal("pubdate", value->name);
	assert_string_equal("Sun, 10 Jan 2010 10:00:00 +0000", value->value);
    
	value = value->next;
	assert_string_equal("items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Sunday, 10 Jan 9:00-10:00</title>"
			"<description>On Sunday 10 January 2010 between 9:00 and 10:00 totals were as follows:&lt;br&gt;Filter 1: 8.00 B &lt;br&gt;Filter 2: 8.00 B &lt;br&gt;The Alert 'alert1' has reached 15.00% of maximum [current: 15.00 B , limit: 100.00 B ].&lt;br&gt;The Alert 'alert2' has reached 8.00% of maximum [current: 8.00 B , limit: 100.00 B ].</description>"
			"<pubDate>Sun, 10 Jan 2010 10:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#hourly.1263114000.1263117600</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 8:00-9:00</title>"
			"<description>On Sunday 10 January 2010 between 8:00 and 9:00 totals were as follows:&lt;br&gt;Filter 1: 4.00 B &lt;br&gt;Filter 2: 0.00 B &lt;br&gt;The Alert 'alert1' has reached 7.00% of maximum [current: 7.00 B , limit: 100.00 B ].&lt;br&gt;The Alert 'alert2' has reached 0.00% of maximum [current: 0.00 B , limit: 100.00 B ].</description>"
			"<pubDate>Sun, 10 Jan 2010 09:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#hourly.1263110400.1263114000</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 7:00-8:00</title>"
			"<description>On Sunday 10 January 2010 between 7:00 and 8:00 totals were as follows:&lt;br&gt;Filter 1: 2.00 B &lt;br&gt;Filter 2: 0.00 B &lt;br&gt;The Alert 'alert1' has reached 3.00% of maximum [current: 3.00 B , limit: 100.00 B ].&lt;br&gt;The Alert 'alert2' has reached 0.00% of maximum [current: 0.00 B , limit: 100.00 B ].</description>"
			"<pubDate>Sun, 10 Jan 2010 08:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#hourly.1263106800.1263110400</guid>"
		"</item>";

	assert_string_equal(itemsXml, value->value);
    
	value = value->next;
	assert_string_equal("ttl", value->name);
	assert_string_equal("60", value->value);
    
	value = value->next;
	assert_string_equal("freq", value->name);
	assert_string_equal("hourly", value->value);
	             
    freeNameValuePairs(firstValue);
	freeStmtList();
}

void testRssWithAlertExpired(void** state) {           
	addFilterRow(FILTER, "Filter 1", "f1", "expr1", NULL);	
    time_t now = makeTs("2010-01-10 10:00:30");
    setTime(now);
    
    addDbRow(makeTs("2010-01-06 06:30:00"), 1,  1, FILTER); // not included (too early)
    addDbRow(makeTs("2010-01-07 07:30:00"), 1,  2, FILTER);
    addDbRow(makeTs("2010-01-08 08:30:00"), 1,  4, FILTER);
    addDbRow(makeTs("2010-01-09 09:30:00"), 1,  8, FILTER);
    addDbRow(makeTs("2010-01-10 09:30:00"), 1, 16, FILTER); // not included (too late)
    
	struct Alert* alert = allocAlert();
    alert->id      = 1;
    alert->active  = 1;
    alert->filter  = 1;
    alert->amount  = 10;
    alert->bound   = makeDateCriteria("2010", "1", "1", "5", "0");
    alert->periods = makeDateCriteria("*", "*", "*", "*", "*");
    setAlertName(alert, "alert1");
    
    addAlert(alert);
    freeAlert(alert);
    
	addConfigRow("web.rss.freq",    "1");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "");
    
    struct NameValuePair* firstValue;
    struct NameValuePair* value = firstValue = makeRssRequestValues();
	assert_string_equal("title", value->name);
	assert_string_equal("BitMeter OS Events", value->value);
    
	value = value->next;
	assert_string_equal("link", value->name);
	assert_string_equal("http://localhost:2605", value->value);
	
	value = value->next;
	assert_string_equal("pubdate", value->name);
	assert_string_equal("Sun, 10 Jan 2010 00:00:00 +0000", value->value);
    
	value = value->next;
	assert_string_equal("items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Saturday 09 January</title>"
			"<description>On Saturday 09 January 2010 totals were as follows: &lt;br&gt;Filter 1: 8.00 B &lt;br&gt;The Alert 'alert1' has exceeded its limit, total is now 150.00% of maximum [current: 15.00 B , limit: 10.00 B ].</description>"
			"<pubDate>Sun, 10 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#daily.1262995200.1263081600</guid>"
		"</item>"
		"<item>"
			"<title>Friday 08 January</title>"
			"<description>On Friday 08 January 2010 totals were as follows: &lt;br&gt;Filter 1: 4.00 B &lt;br&gt;The Alert 'alert1' has reached 70.00% of maximum [current: 7.00 B , limit: 10.00 B ].</description>"
			"<pubDate>Sat, 09 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#daily.1262908800.1262995200</guid>"
		"</item>"
		"<item>"
			"<title>Thursday 07 January</title>"
			"<description>On Thursday 07 January 2010 totals were as follows: &lt;br&gt;Filter 1: 2.00 B &lt;br&gt;The Alert 'alert1' has reached 30.00% of maximum [current: 3.00 B , limit: 10.00 B ].</description>"
			"<pubDate>Fri, 08 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#daily.1262822400.1262908800</guid>"
		"</item>";
    
	assert_string_equal(itemsXml, value->value);
    
	value = value->next;
	assert_string_equal("ttl", value->name);
	assert_string_equal("1440", value->value);
    
	value = value->next;
	assert_string_equal("freq", value->name);
	assert_string_equal("daily", value->value);
	
    freeNameValuePairs(firstValue);
	freeStmtList();
}

void testRssDailyNoAlerts(void** state) {            
	addFilterRow(FILTER, "Filter 1", "f1", "expr1", NULL);	
	addDbRow(makeTs("2010-01-06 05:30:00"), 1, 1, FILTER); // not included (too early)
    addDbRow(makeTs("2010-01-07 06:30:00"), 1, 1, FILTER);
    addDbRow(makeTs("2010-01-08 07:30:00"), 1, 2, FILTER);
    addDbRow(makeTs("2010-01-09 08:30:00"), 1, 4, FILTER);
    addDbRow(makeTs("2010-01-10 09:30:00"), 1, 8, FILTER); // not included (too late)
    
	addConfigRow("web.rss.freq",    "1");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "My Server");
	addConfigRow("web.rss.host",    "myhost");
	addConfigRow("web.port",        "1234");
    
    time_t now = makeTs("2010-01-10 10:00:01");
    setTime(now);
    
    struct NameValuePair* firstValue;
    struct NameValuePair* value = firstValue = makeRssRequestValues();
	assert_string_equal("title", value->name);
	assert_string_equal("BitMeter OS Events - My Server", value->value);
	
	value = value->next;
	assert_string_equal("link", value->name);
	assert_string_equal("http://myhost:1234", value->value);
	
	value = value->next;
	assert_string_equal("pubdate", value->name);
	assert_string_equal("Sun, 10 Jan 2010 00:00:00 +0000", value->value);
    
	value = value->next;
	assert_string_equal("items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Saturday 09 January</title>"
			"<description>On Saturday 09 January 2010 totals were as follows: &lt;br&gt;Filter 1: 4.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://myhost:1234/#daily.1262995200.1263081600</guid>"
		"</item>"
		"<item>"
			"<title>Friday 08 January</title>"
			"<description>On Friday 08 January 2010 totals were as follows: &lt;br&gt;Filter 1: 2.00 B </description>"
			"<pubDate>Sat, 09 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://myhost:1234/#daily.1262908800.1262995200</guid>"
		"</item>"
		"<item>"
			"<title>Thursday 07 January</title>"
			"<description>On Thursday 07 January 2010 totals were as follows: &lt;br&gt;Filter 1: 1.00 B </description>"
			"<pubDate>Fri, 08 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://myhost:1234/#daily.1262822400.1262908800</guid>"
		"</item>";
    
	assert_string_equal(itemsXml, value->value);
    
	value = value->next;
	assert_string_equal("ttl", value->name);
	assert_string_equal("1440", value->value);
    
	value = value->next;
	assert_string_equal("freq", value->name);
	assert_string_equal("daily", value->value);    
	
    freeNameValuePairs(firstValue);
	freeStmtList();
}

