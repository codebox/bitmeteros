#ifdef UNIT_TESTING
	#include "test.h"
#endif
#include <stdio.h>
#include <time.h>
#include "bmws.h"
#include "client.h"
#include "common.h"

/*
Handles '/rss.xml' requests received by the web server.
*/

static void getHourlyItems(char** itemsTxt, int rssItemCount, char* guidBase);
static void getDailyItems(char** itemsTxt, int rssItemCount, char* guidBase);
static char* getPubDate(int rssFreq);
static char* getAlertsTxt(time_t now);
struct NameValuePair* makeRssRequestValues();

#define RSS_FREQ_DAILY  1
#define RSS_FREQ_HOURLY 2
#define DEFAULT_RSS_ITEM_COUNT 10
#define FEED_TITLE "BitMeter OS Events"
#define HTML_LINE_BREAK "&lt;br&gt;"

void processRssRequest(SOCKET fd, struct Request* req){
 /* The skeleton XML returned by the RSS feed is contained in the 'rss.xml' file in
 	the root of the web folder, the placeholder comments of the form <!--[something]-->
 	are replaced by values that we calculate below. */
	struct NameValuePair* values = makeRssRequestValues();

	processFileRequest(fd, req, values);
	freeNameValuePairs(values);
}

struct NameValuePair* makeRssRequestValues(){
 // If a custom server name has been specified then we include it in the title of the feed
	char* serverName = getConfigText(CONFIG_WEB_SERVER_NAME, TRUE);
	char* title;
	if ((serverName != NULL) && (strlen(serverName) > 0)){
		title = malloc(strlen(FEED_TITLE) + 3 + strlen(serverName) + 1);
		sprintf(title, "%s - %s", FEED_TITLE, serverName);
	} else {
		title = strdup(FEED_TITLE);
	}
	if (serverName != NULL){
		free(serverName);
	}

 /* The feed needs a site url, if the url must work when accessed remotely then the hostname
 	must be specfied in the config table - check if it's there... */
	char* rssHost = getConfigText(CONFIG_WEB_RSS_HOST, TRUE);
	if ((rssHost == NULL) || (strlen(rssHost) == 0)){
		if (rssHost != NULL){
			free(rssHost);
		}
		rssHost	= strdup("localhost"); // use 'localhost' as the default host name for the feed
	}
	int rssPort = getConfigInt(CONFIG_WEB_PORT, TRUE);
	if (rssPort < MIN_PORT || rssPort > MAX_PORT){
		rssPort = DEFAULT_PORT; // No custom port was specified
	}

 // Build a url for the site using the host and port
	char rssUrl[7 + strlen(rssHost) + 1 + 5 + 1];
	sprintf(rssUrl, "http://%s:%d", rssHost, rssPort);
	if (rssHost != NULL){
		free(rssHost);
	}

 // How often we publish (hourly or daily)
	int rssFreq = getConfigInt(CONFIG_WEB_RSS_FREQ, TRUE);
	if ((rssFreq != RSS_FREQ_DAILY) && (rssFreq != RSS_FREQ_HOURLY)){
		rssFreq	= RSS_FREQ_DAILY;
	}

 // How many items appear in the feed
	int rssItemCount = getConfigInt(CONFIG_WEB_RSS_ITEMS, TRUE);
	if (rssItemCount < 1){
		rssItemCount = DEFAULT_RSS_ITEM_COUNT;
	}

 // Fill this array with the XML <item> elements that we will include in the feed
	char* items[rssItemCount];
	if (rssFreq == RSS_FREQ_HOURLY){
		getHourlyItems(items, rssItemCount, rssUrl);
	} else {
		getDailyItems(items, rssItemCount, rssUrl);
	}

 // Calculate how much space we need for all the items
	int i, size=0;
	for(i=0; i<rssItemCount; i++){
		size += strlen(items[i]);
	}

 // Populate the itemsTxt buffer with all the text
	char* itemsTxt = malloc(size+1);
	int j, offset=0;
	char* itemTxt;
	for(j=0; j<rssItemCount; j++){
		itemTxt = items[j];
		strcpy(itemsTxt + offset, itemTxt);
		offset += strlen(itemTxt);
		free(itemTxt);
	}
	itemsTxt[size] = 0;

 // Publication date for the feed, in the correct format
	char* pubDate = getPubDate(rssFreq);

 // Refresh interval for the feed in minutes
	char* ttlTxt = (rssFreq == RSS_FREQ_HOURLY) ? "60" : "1440";

 // Textual frequency of updates
	char* freqTxt = (rssFreq == RSS_FREQ_HOURLY) ? "hourly" : "daily";

 // These 6 values get substituted into the XML from the static file
	struct NameValuePair* pair = makeNameValuePair("title", title);
	appendNameValuePair(&pair, makeNameValuePair("link",    rssUrl));
	appendNameValuePair(&pair, makeNameValuePair("pubdate", pubDate));
	appendNameValuePair(&pair, makeNameValuePair("items",   itemsTxt));
	appendNameValuePair(&pair, makeNameValuePair("ttl",     ttlTxt));
	appendNameValuePair(&pair, makeNameValuePair("freq",    freqTxt));

 // We can free these because the values were copied in makeNameValuePair calls above
    free(title);
    free(pubDate);
    free(itemsTxt);

    return pair;
}
#ifdef _WIN32
 // Timezone not formatted correctly on Windows using '%z' so we have to roll our own
	static void getRfc822Time(struct tm* time, char* timeTxt){
		char part1[48];
		strftime(part1, 47, "%a, %d %b %Y %H:%M:%S", time);

		TIME_ZONE_INFORMATION info;
		int ret = GetTimeZoneInformation(&info);

		int bias;
		if (ret == TIME_ZONE_ID_STANDARD || ret == TIME_ZONE_ID_UNKNOWN){
      		bias = -info.StandardBias;
   		} else if (ret == TIME_ZONE_ID_DAYLIGHT){
   			bias = -info.DaylightBias;
   		} else {
   			bias = 0;
   			//logMsg(LOG_ERR, "Unable to retrieve timezone information, rc=%d", ret);
   		}
   		int hh = bias / 60;
		int mm = bias % 60;
		int plus = hh >= 0;

		sprintf(timeTxt, "%s %c%02d%02d", part1, (plus ? '+' : '-'), abs(hh), mm);
	}
#endif
#ifndef _WIN32
	static void getRfc822Time(struct tm* time, char* timeTxt){
		strftime(timeTxt, 47, "%a, %d %b %Y %H:%M:%S %z", time);
	}
#endif

static char* makeItem(char* title, char* amountDescription, char* alertDescription, struct tm* pubTime,
		char* guid){
 // Build the <item>...</item> XML for a single item in the feed

 // Publication time for the item
	char pubTimeTxt[48];
	getRfc822Time(pubTime, pubTimeTxt);

	if (alertDescription == NULL){
		alertDescription = "";
	}

 // XML gets written into here
	char* itemTxt = malloc(6 + 7 + strlen(title) + 8 + 13 + strlen(amountDescription) +
			strlen(HTML_LINE_BREAK) + strlen(alertDescription) + 14 + 9 + strlen(pubTimeTxt) +
			10 + 27 + strlen(guid) + 7 + 7 + 1);

	sprintf(itemTxt,
			"<item>"
				"<title>%s</title>"
				"<description>%s%s%s</description>"
				"<pubDate>%s</pubDate>"
				"<guid isPermaLink=\"false\">%s</guid>"
			"</item>",
			title, amountDescription,
			strlen(alertDescription) > 0 ? HTML_LINE_BREAK : "",
			alertDescription, pubTimeTxt, guid);

	return itemTxt;
}

static char* getAmountDesc(struct Filter* filters, time_t from, time_t to) {
	struct Filter* thisFilter;
	struct Filter* filter = filters;
	int filterCount = 0;

	while(filter != NULL){
		filter = filter->next;
		filterCount++;
	}

 // Allocate space for the string pointers
	char** amountTxt = malloc(filterCount * sizeof(char*));
	int i=0;

 // Run the query for each filter
 	filter = filters;
	while(filter != NULL){
		struct Data* totals = getQueryValues(from, to, QUERY_GROUP_TOTAL, filter->id);
		if (totals == NULL){
			totals = allocData();
		}
		char* amtAndFilter = malloc(strlen(HTML_LINE_BREAK) + 24 + 2 + strlen(filter->desc) + 1);
		char amt[24];
		formatAmount(totals->vl, TRUE, TRUE, amt);
		sprintf(amtAndFilter, "%s%s: %s", (i==0 ? "" : HTML_LINE_BREAK), filter->desc, amt);
		freeData(totals);

		amountTxt[i++] = amtAndFilter;
		filter = filter->next;
	}

	char* descAmt = malloc(24 * filterCount + 64 + 64);
	int offset=0, amtLen;

	for (i=0; i<filterCount; i++) {
		amtLen = strlen(amountTxt[i]);
		strcpy(descAmt + offset, amountTxt[i]);

		free(amountTxt[i]);
		offset += amtLen;

	}
	descAmt[offset] = 0;

	free(amountTxt);

	return descAmt;
}

static void getDailyItems(char** itemsTxt, int rssItemCount, char* rssUrl){
 /* Populate the itemsTxt array with the various XML strings, each string contains an
 	item with information about upload/download volumes for a single day. */
	struct tm toStruct = getLocalTime(getTime());

	toStruct.tm_sec = 0;
	toStruct.tm_min = 0;
	toStruct.tm_hour = 0;

	int count=0;
	time_t from, to;
	char itemTitle[24]; // The title of the item, which will contain just a date
	char descDate[64];	// Publication date for the item, in the correct format

	struct Filter* filters = readFilters();

	while (count<rssItemCount){
	 // Calculate the upper/lower bounds that correspond to the day we are working on
		to = mktime(&toStruct);
		toStruct.tm_mday -= 1;
		from = mktime(&toStruct);

	 // The title of the item will be the day from the start of the query range
		strftime(itemTitle, 23, "%A %d %B", &toStruct);

		char* amounts = getAmountDesc(filters, from, to);

	 // The descriptive text also contains the date, in a more verbose format
		strftime(descDate, 63, "%A %d %B %Y", &toStruct);

	 /* Construct the descriptive text, its HTML but we must escape the entities so it
	 	doesn't break the XML structure */
	 	char descAmt[strlen(amounts) + 128];
		sprintf(descAmt, "On %s totals were as follows: %s%s", descDate, HTML_LINE_BREAK, amounts);

	 // If there are any Alerts defined then get some text describing their status
		char* descAlerts = getAlertsTxt(to);

	 /* Build a unique ID (guid) to identify this item, this will be the same for a given
	 	day every time the feed is requested, to ensure that aggregators don't display
	 	duplicates */
		char guid[strlen(rssUrl) + 32];
		sprintf(guid, "%s/#daily.%d.%d", rssUrl, (int)from, (int)to);

		toStruct.tm_mday += 1;
		normaliseTm(&toStruct);
		char* itemTxt = makeItem(itemTitle, descAmt, descAlerts, &toStruct, guid);

		toStruct.tm_mday -= 1;
		itemsTxt[count++] = itemTxt;

		if (descAlerts != NULL){
			free(descAlerts);
		}
		free(amounts);
	}
	
	freeFilters(filters);
}

static void getHourlyItems(char** itemsTxt, int rssItemCount, char* rssUrl){
 /* Populate the itemsTxt array with the various XML strings, each string contains an
 	item with information about upload/download volumes for a single hour. */
	struct tm toStruct = getLocalTime(getTime());

	toStruct.tm_sec = 0;
	toStruct.tm_min = 0;

	int count=0;
	time_t from, to;
	char itemTitle[64];	// The title of the item, which will contain a date, and a time range
	char titleDate[24];	// The date, as it appears in the item title
	char descDate[64];	// Publication date for the item, in the correct format

	struct Filter* filters = readFilters();

	while (count<rssItemCount){
	 // Calculate the upper/lower bounds that correspond to the hour we are working on
		to = mktime(&toStruct);
		toStruct.tm_hour -= 1;
		from = mktime(&toStruct);

	 // The title of the item will be the day, followed by the time range coveed by the item
		strftime(titleDate, 23, "%A, %d %b", &toStruct);
		sprintf(itemTitle, "%s %d:00-%d:00", titleDate, toStruct.tm_hour, toStruct.tm_hour+1);

	 // Run the querys
		char* amounts = getAmountDesc(filters, from, to);

	 // The descriptive text also contains the date, in a more verbose format
		strftime(descDate, 63, "%A %d %B %Y", &toStruct);

	 /* Construct the descriptive text, its HTML but we must escape the entities so it
	 	doesn't break the XML structure */
	 	char descAmt[strlen(amounts) + 128];
		sprintf(descAmt, "On %s between %d:00 and %d:00 totals were as follows:%s%s",
				descDate, toStruct.tm_hour, toStruct.tm_hour+1, HTML_LINE_BREAK, amounts);

	 // If there are any Alerts defined then get some text describing their status
		char* descAlerts = getAlertsTxt(to);

	 /* Build a unique ID (guid) to identify this item, this will be the same for a given
	 	time/day every time the feed is requested, to ensure that aggregators don't display
	 	duplicates */
		char guid[strlen(rssUrl) + 32];
		sprintf(guid, "%s/#hourly.%d.%d", rssUrl, (int)from, (int)to);

		toStruct.tm_hour += 1;
		char* itemTxt = makeItem(itemTitle, descAmt, descAlerts, &toStruct, guid);
		toStruct.tm_hour -= 1;

		itemsTxt[count++] = itemTxt;
		if (descAlerts != NULL){
			free(descAlerts);
		}
		free(amounts);
	}
	
	freeFilters(filters);
}

static char* getPubDate(int rssFreq){
 // Build a string containing the correctly formatted date/time of the current publication date for the feed
	struct tm now = getLocalTime(getTime());
	now.tm_sec = 0;
	now.tm_min = 0;
	if (rssFreq == RSS_FREQ_DAILY){
		now.tm_hour = 0;
	}

	char pubDateTxt[48];
	getRfc822Time(&now, pubDateTxt);

	return strdup(pubDateTxt);
}

static char* getAlertsTxt(time_t now){
 // Builds text describing the status of any alerts that have been defined, as they were at	specified date/time
	struct Data* totals;

	struct Alert* firstAlert = getAlerts();
	struct Alert* alert = firstAlert;

	char* alertsText = NULL; // This is where the text goes
	while(alert != NULL) {
		totals = getTotalsForAlert(alert, now);
		BW_INT total = totals->vl;
    	freeData(totals);

	 // The first part of the text for an Alert contains the name
		char alertTextPart1[strlen(alert->name) + 14];
		sprintf(alertTextPart1, "The Alert '%s'", alert->name);

	 // The second part of the text contains the status of the Alert
		double progress = (double)100 * total / alert->amount;
		char alertTextPart2[64];
    	if (total > alert->amount){
    		sprintf(alertTextPart2, "has exceeded its limit, total is now %1.2f%% of maximum", progress);
    	} else {
    		sprintf(alertTextPart2, "has reached %1.2f%% of maximum", progress);
    	}

	 // The third part of the text contains the current/maximum amounts
		char limitAmt[16];
		formatAmount(alert->amount, TRUE, TRUE, limitAmt);
		char currentAmt[16];
		formatAmount(total, TRUE, TRUE, currentAmt);
		char alertTextPart3[64];
		sprintf(alertTextPart3, "[current: %s, limit: %s].", currentAmt, limitAmt);

	 // Join the parts together into a single string
		int alertTextLen = strlen(alertTextPart1) + 1 + strlen(alertTextPart2) + 1 + strlen(alertTextPart3);
		char alertText[alertTextLen + 1];
		sprintf(alertText, "%s %s %s", alertTextPart1, alertTextPart2, alertTextPart3);

	 // Add the new string into the 'alertsText' value that will be returned
		if (alertsText==NULL){
			alertsText = strdup(alertText); // This is the first Alert, so alertsText is NULL

		} else {
		 // This is not the first Alert, so append the new text onto the existing text
			char* newAlertsTxt = malloc(strlen(alertsText) + strlen(HTML_LINE_BREAK) + alertTextLen + 1);
			strcpy(newAlertsTxt, alertsText);
			strcat(newAlertsTxt, HTML_LINE_BREAK);
			strcat(newAlertsTxt, alertText);
			free(alertsText);

			alertsText = newAlertsTxt;
		}

		alert = alert->next;
	}
	freeAlert(firstAlert);

	return alertsText;
}


