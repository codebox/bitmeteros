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
#include "common.h"
#include "test.h"
#include "client.h"
#include "CuTest.h"
#include "bmws.h"

/*
Contains unit tests for the handleRss module.
*/

void testRssHourlyNoAlerts(CuTest *tc) {
    emptyDb();
    addDbRow(makeTs("2010-01-10 06:30:00"), 1, NULL,  1,  1, ""); // not included (too early)
    addDbRow(makeTs("2010-01-10 07:30:00"), 1, NULL,  2,  2, "");
    addDbRow(makeTs("2010-01-10 08:30:00"), 1, NULL,  4,  4, "");
    addDbRow(makeTs("2010-01-10 09:30:00"), 1, NULL,  8,  8, "");
    addDbRow(makeTs("2010-01-10 10:00:10"), 1, NULL, 16, 16, ""); // not included (too late)

	addConfigRow("web.rss.host",    "rsshost");
	addConfigRow("web.rss.freq",    "2");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "");

    time_t now = makeTs("2010-01-10 10:00:30");
    setTime(now);

    struct NameValuePair* value = makeRssRequestValues();
	CuAssertStrEquals(tc, "title", value->name);
	CuAssertStrEquals(tc, "BitMeter OS Events", value->value);
	
	value = value->next;
	CuAssertStrEquals(tc, "link", value->name);
	CuAssertStrEquals(tc, "http://rsshost:2605", value->value);
	
	value = value->next;
	CuAssertStrEquals(tc, "pubdate", value->name);
	CuAssertStrEquals(tc, "Sun, 10 Jan 2010 10:00:00 +0000", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Sunday, 10 Jan 9:00-10:00</title>"
			"<description>On Sunday 10 January 2010  between 9:00 and 10:00 totals were as follows:&lt;br&gt;Download: 8.00 B &lt;br&gt;Upload: 8.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 10:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://rsshost:2605/#hourly.1263114000.1263117600</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 8:00-9:00</title>"
			"<description>On Sunday 10 January 2010  between 8:00 and 9:00 totals were as follows:&lt;br&gt;Download: 4.00 B &lt;br&gt;Upload: 4.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 09:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://rsshost:2605/#hourly.1263110400.1263114000</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 7:00-8:00</title>"
			"<description>On Sunday 10 January 2010  between 7:00 and 8:00 totals were as follows:&lt;br&gt;Download: 2.00 B &lt;br&gt;Upload: 2.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 08:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://rsshost:2605/#hourly.1263106800.1263110400</guid>"
		"</item>";
		
	CuAssertStrEquals(tc, itemsXml, value->value);

	value = value->next;
	CuAssertStrEquals(tc, "ttl", value->name);
	CuAssertStrEquals(tc, "60", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "freq", value->name);
	CuAssertStrEquals(tc, "hourly", value->value);
}

void testRssWithAlertOk(CuTest *tc) {
    time_t now = makeTs("2010-01-10 10:00:30");
    setTime(now);

    emptyDb();
    addDbRow(makeTs("2010-01-10 06:30:00"), 1, NULL,  1,  1, ""); // not included (too early)
    addDbRow(makeTs("2010-01-10 07:30:00"), 1, NULL,  2,  2, "");
    addDbRow(makeTs("2010-01-10 08:30:00"), 1, NULL,  4,  4, "");
    addDbRow(makeTs("2010-01-10 09:30:00"), 1, NULL,  8,  8, "");
    addDbRow(makeTs("2010-01-10 10:00:10"), 1, NULL, 16, 16, ""); // not included (too late)

	struct Alert* alert = allocAlert();
    alert->id        = 1;
    alert->active    = 1;
    alert->direction = 1;
    alert->amount    = 100;
    alert->bound     = makeDateCriteria("2010", "1", "1", "5", "0");
    setAlertName(alert, "alert1");
    
    addAlert(alert);
	
	addConfigRow("web.rss.host",    "");
	addConfigRow("web.rss.freq",    "2");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "");

    struct NameValuePair* value = makeRssRequestValues();
	CuAssertStrEquals(tc, "title", value->name);
	CuAssertStrEquals(tc, "BitMeter OS Events", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "link", value->name);
	CuAssertStrEquals(tc, "http://localhost:2605", value->value);
	
	value = value->next;
	CuAssertStrEquals(tc, "pubdate", value->name);
	CuAssertStrEquals(tc, "Sun, 10 Jan 2010 10:00:00 +0000", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Sunday, 10 Jan 9:00-10:00</title>"
			"<description>"
				"On Sunday 10 January 2010  between 9:00 and 10:00 totals were as follows:&lt;br&gt;Download: 8.00 B &lt;br&gt;Upload: 8.00 B "
				"&lt;br&gt;The Alert 'alert1' has reached 15.00% of maximum [current: 15.00 B , limit: 100.00 B ]."
			"</description>"
			"<pubDate>Sun, 10 Jan 2010 10:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#hourly.1263114000.1263117600</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 8:00-9:00</title>"
			"<description>"
				"On Sunday 10 January 2010  between 8:00 and 9:00 totals were as follows:&lt;br&gt;Download: 4.00 B &lt;br&gt;Upload: 4.00 B "
				"&lt;br&gt;The Alert 'alert1' has reached 7.00% of maximum [current: 7.00 B , limit: 100.00 B ]."
			"</description>"
			"<pubDate>Sun, 10 Jan 2010 09:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#hourly.1263110400.1263114000</guid>"
		"</item>"
		"<item>"
			"<title>Sunday, 10 Jan 7:00-8:00</title>"
			"<description>"
				"On Sunday 10 January 2010  between 7:00 and 8:00 totals were as follows:&lt;br&gt;Download: 2.00 B &lt;br&gt;Upload: 2.00 B "
				"&lt;br&gt;The Alert 'alert1' has reached 3.00% of maximum [current: 3.00 B , limit: 100.00 B ]."
			"</description>"
			"<pubDate>Sun, 10 Jan 2010 08:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#hourly.1263106800.1263110400</guid>"
		"</item>";
		
	CuAssertStrEquals(tc, itemsXml, value->value);

	value = value->next;
	CuAssertStrEquals(tc, "ttl", value->name);
	CuAssertStrEquals(tc, "60", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "freq", value->name);
	CuAssertStrEquals(tc, "hourly", value->value);
}

void testRssWithAlertExpired(CuTest *tc) {
    time_t now = makeTs("2010-01-10 10:00:30");
    setTime(now);

    emptyDb();
    addDbRow(makeTs("2010-01-06 06:30:00"), 1, NULL,  1,  1, ""); // not included (too early)
    addDbRow(makeTs("2010-01-07 07:30:00"), 1, NULL,  2,  2, "");
    addDbRow(makeTs("2010-01-08 08:30:00"), 1, NULL,  4,  4, "");
    addDbRow(makeTs("2010-01-09 09:30:00"), 1, NULL,  8,  8, "");
    addDbRow(makeTs("2010-01-10 09:30:00"), 1, NULL, 16, 16, ""); // not included (too late)

	struct Alert* alert = allocAlert();
    alert->id        = 1;
    alert->active    = 1;
    alert->direction = 1;
    alert->amount    = 10;
    alert->bound     = makeDateCriteria("2010", "1", "1", "5", "0");
    alert->periods   = makeDateCriteria("*", "*", "*", "*", "*");
    setAlertName(alert, "alert1");
    
    addAlert(alert);
    
	addConfigRow("web.rss.freq",    "1");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "");

    struct NameValuePair* value = makeRssRequestValues();
	CuAssertStrEquals(tc, "title", value->name);
	CuAssertStrEquals(tc, "BitMeter OS Events", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "link", value->name);
	CuAssertStrEquals(tc, "http://localhost:2605", value->value);
	
	value = value->next;
	CuAssertStrEquals(tc, "pubdate", value->name);
	CuAssertStrEquals(tc, "Sun, 10 Jan 2010 00:00:00 +0000", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Saturday 09 January</title>"
			"<description>"
				"On Saturday 09 January 2010 totals were as follows:&lt;br&gt;Download: 8.00 B &lt;br&gt;Upload: 8.00 B "
				"&lt;br&gt;The Alert 'alert1' has exceeded its limit, total is now 150.00% of maximum [current: 15.00 B , limit: 10.00 B ]."
			"</description>"
			"<pubDate>Sun, 10 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#daily.1262995200.1263081600</guid>"
		"</item>"
		"<item>"
			"<title>Friday 08 January</title>"
			"<description>"
				"On Friday 08 January 2010 totals were as follows:&lt;br&gt;Download: 4.00 B &lt;br&gt;Upload: 4.00 B "
				"&lt;br&gt;The Alert 'alert1' has reached 70.00% of maximum [current: 7.00 B , limit: 10.00 B ]."
			"</description>"
			"<pubDate>Sat, 09 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#daily.1262908800.1262995200</guid>"
		"</item>"
		"<item>"
			"<title>Thursday 07 January</title>"
			"<description>"
				"On Thursday 07 January 2010 totals were as follows:&lt;br&gt;Download: 2.00 B &lt;br&gt;Upload: 2.00 B "
				"&lt;br&gt;The Alert 'alert1' has reached 30.00% of maximum [current: 3.00 B , limit: 10.00 B ]."
			"</description>"
			"<pubDate>Fri, 08 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://localhost:2605/#daily.1262822400.1262908800</guid>"
		"</item>";
		
	CuAssertStrEquals(tc, itemsXml, value->value);

	value = value->next;
	CuAssertStrEquals(tc, "ttl", value->name);
	CuAssertStrEquals(tc, "1440", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "freq", value->name);
	CuAssertStrEquals(tc, "daily", value->value);
}

void testRssDailyNoAlerts(CuTest *tc) {
	emptyDb();
	addDbRow(makeTs("2010-01-06 05:30:00"), 1, NULL,  1,  1, ""); // not included (too early)
    addDbRow(makeTs("2010-01-07 06:30:00"), 1, NULL,  1,  1, "");
    addDbRow(makeTs("2010-01-08 07:30:00"), 1, NULL,  2,  2, "");
    addDbRow(makeTs("2010-01-09 08:30:00"), 1, NULL,  4,  4, "");
    addDbRow(makeTs("2010-01-10 09:30:00"), 1, NULL,  8,  8, ""); // not included (too late)

	addConfigRow("web.rss.freq",    "1");
	addConfigRow("web.rss.items",   "3");
	addConfigRow("web.server_name", "My Server");
	addConfigRow("web.rss.host",    "myhost");
	addConfigRow("web.port",        "1234");

    time_t now = makeTs("2010-01-10 10:00:01");
    setTime(now);

    struct NameValuePair* value = makeRssRequestValues();
	CuAssertStrEquals(tc, "title", value->name);
	CuAssertStrEquals(tc, "BitMeter OS Events - My Server", value->value);
	
	value = value->next;
	CuAssertStrEquals(tc, "link", value->name);
	CuAssertStrEquals(tc, "http://myhost:1234", value->value);
	
	value = value->next;
	CuAssertStrEquals(tc, "pubdate", value->name);
	CuAssertStrEquals(tc, "Sun, 10 Jan 2010 00:00:00 +0000", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "items", value->name);
	char* itemsXml = 
		"<item>"
			"<title>Saturday 09 January</title>"
			"<description>On Saturday 09 January 2010 totals were as follows:&lt;br&gt;Download: 4.00 B &lt;br&gt;Upload: 4.00 B </description>"
			"<pubDate>Sun, 10 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://myhost:1234/#daily.1262995200.1263081600</guid>"
		"</item>"
		"<item>"
			"<title>Friday 08 January</title>"
			"<description>On Friday 08 January 2010 totals were as follows:&lt;br&gt;Download: 2.00 B &lt;br&gt;Upload: 2.00 B </description>"
			"<pubDate>Sat, 09 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://myhost:1234/#daily.1262908800.1262995200</guid>"
		"</item>"
		"<item>"
			"<title>Thursday 07 January</title>"
			"<description>On Thursday 07 January 2010 totals were as follows:&lt;br&gt;Download: 1.00 B &lt;br&gt;Upload: 1.00 B </description>"
			"<pubDate>Fri, 08 Jan 2010 00:00:00 +0000</pubDate>"
			"<guid isPermaLink=\"false\">http://myhost:1234/#daily.1262822400.1262908800</guid>"
		"</item>";
		
	CuAssertStrEquals(tc, itemsXml, value->value);

	value = value->next;
	CuAssertStrEquals(tc, "ttl", value->name);
	CuAssertStrEquals(tc, "1440", value->value);

	value = value->next;
	CuAssertStrEquals(tc, "freq", value->name);
	CuAssertStrEquals(tc, "daily", value->value);    
}

CuSuite* handleRssGetSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testRssHourlyNoAlerts);
    SUITE_ADD_TEST(suite, testRssWithAlertOk);
    SUITE_ADD_TEST(suite, testRssWithAlertExpired);
    SUITE_ADD_TEST(suite, testRssDailyNoAlerts);

    return suite;
}
