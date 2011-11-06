#include <stdlib.h>
#include <stdio.h>
#include "bmws.h"
#include "client.h"
#include "common.h"

#define HOST_SQL "SELECT DISTINCT(host) FROM filter WHERE host IS NOT NULL"
#define WEB_SERVER_NAME_MAX_LEN 32
#define RSS_HOST_NAME_MAX_LEN   32
#define BAD_NUM -1
#define RSS_ITEMS_MIN  1
#define RSS_ITEMS_MAX 20
#define RSS_FREQ_MIN   1
#define RSS_FREQ_MAX   2
#define COLOUR_LEN     6
#define MONITOR_INTERVAL_MIN 1000
#define MONITOR_INTERVAL_MAX 30000
#define HISTORY_INTERVAL_MIN 5000
#define HISTORY_INTERVAL_MAX 60000
#define SUMMARY_INTERVAL_MIN 1000
#define SUMMARY_INTERVAL_MAX 60000

/*
Handles '/config' requests received by the web server.
*/

static void writeConfig(SOCKET fd, int allowAdmin);
static void writeNumConfigValue(SOCKET fd, char* key, char* value);
static void writeNumConfigNumValue(SOCKET fd, char* key, int value);
static void writeTxtConfigValue(SOCKET fd, char* key, char* value);
static void writeHostList(SOCKET fd);
static void writeFilterList(SOCKET fd);
static int updateWebServerName(char* value);
static int updateRssHostName(char* value);
static int updateRssFreq(char* value);
static int updateRssItems(char* value);
static int updateColour(char* name, char* value);
static int updateMonitorInterval(char* value);
static int updateHistoryInterval(char* value);
static int updateSummaryInterval(char* value);

void processConfigRequest(SOCKET fd, struct Request* req, int allowAdmin){
    struct NameValuePair* params = req->params;

    if (params == NULL){
     // If there are no request parameters then we send back the current config as a JSON object
        writeConfig(fd, allowAdmin);

    } else {
     // If there are parameters then this is a config-update request
        if (allowAdmin){
            int status = SUCCESS;
            while(params != NULL){
             // Check which config value we are updating
                if (strcmp(CONFIG_WEB_SERVER_NAME, params->name) == 0){
                    status = updateWebServerName(params->value);

                } else if (strcmp(CONFIG_WEB_RSS_HOST, params->name) == 0){
                    status = updateRssHostName(params->value);

                } else if (strcmp(CONFIG_WEB_MONITOR_INTERVAL, params->name) == 0){
                    status = updateMonitorInterval(params->value);

                } else if (strcmp(CONFIG_WEB_HISTORY_INTERVAL, params->name) == 0){
                    status = updateHistoryInterval(params->value);

                } else if (strcmp(CONFIG_WEB_SUMMARY_INTERVAL, params->name) == 0){
                    status = updateSummaryInterval(params->value);

                } else if (strcmp(CONFIG_WEB_RSS_FREQ, params->name) == 0){
                    status = updateRssFreq(params->value);

                } else if (strcmp(CONFIG_WEB_RSS_ITEMS, params->name) == 0){
                    status = updateRssItems(params->value);

                } else if (strstr(params->name, CONFIG_WEB_COLOUR) == params->name) {
                    status = updateColour(params->name, params->value);

                } else {
                    if (strcmp("_", params->name) == 0){
                     // This parameter gets tagged onto every AJAX request from jQuery to prevent caching issues in IE

                    } else {
                     // Not all configs can be updated via the web
                        logMsg(LOG_ERR, "Update request for illegal/unknown config param: %s=%s", params->name, params->value);
                        status = FAIL;
                    }
                }

                if (status == FAIL){
                    break;
                }
                params = params->next;
            }

            if (status == SUCCESS){
                WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);
                WRITE_TEXT(fd, "{}");
            } else {
                WRITE_HEADERS_SERVER_ERROR(fd, "Config update failed %s=%s", params->name, params->value); 
            }

        } else {
         // Config updates are an administrative operation
            WRITE_HEADERS_FORBIDDEN(fd, "config update");
        }
    }
}

static int isValueLenOk(char* value, int maxlen){
    if (strlen(value) > maxlen) {
        logMsg(LOG_ERR, "Attempt to change config value to %s - value too long (maxlen=%d)", value, maxlen);
        return FALSE;

    } else {
        return TRUE;
    }
}
static int isNumericOk(char* value, int minValue, int maxValue){
    long longVal = strToLong(value, BAD_NUM);
    if ((longVal == BAD_NUM) || (longVal < minValue) || (longVal > maxValue)) {
        logMsg(LOG_ERR, "Attempt to update numeric config with invalid/out-of-range value of %s (min=%d max=%d)",
            value, minValue, maxValue);
        return FALSE;

    } else {
        return TRUE;
    }
}
static int isColourOk(char* value){
    if (strlen(value) != COLOUR_LEN) {
        logMsg(LOG_ERR, "Attempt to change config colour value to %s - length must be %d", value, COLOUR_LEN);
        return FALSE;

    } else {
        int i;
        char c; //TODO use regex
        for(i=0; i<COLOUR_LEN; i++){
            c = value[i];
            if (!((c>='0' && c<='9') || (c>='a' && c<='f'))){
                logMsg(LOG_ERR, "Attempt to change config colour value to %s - characters must be 0-9 or a-f.", value);
                return FALSE;
            }
        }
    }
    return TRUE;
}


static int isFilterNameOk(char* filterName){
    struct Filter* allFilters = readFilters();
    struct Filter* filter = getFilterFromName(allFilters, filterName, NULL);
    int filterNameOk = (filter != NULL);
    freeFilters(allFilters);
    
    return filterNameOk;
}
static int noDodgyChars(char* value){
    if ((strchr(value, '<') != NULL) || (strchr(value, '>') != NULL)) {
        logMsg(LOG_ERR, "Suspicious characters detected in config value %s", value);
        return FALSE;

    } else {
        return TRUE;
    }
}

static int updateMonitorInterval(char* value){
    if (isNumericOk(value, MONITOR_INTERVAL_MIN, MONITOR_INTERVAL_MAX)) {
        setConfigTextValue(CONFIG_WEB_MONITOR_INTERVAL, value);
        return SUCCESS;

    } else {
        return FAIL;
    }
}
static int updateHistoryInterval(char* value){
    if (isNumericOk(value, HISTORY_INTERVAL_MIN, HISTORY_INTERVAL_MAX)) {
        setConfigTextValue(CONFIG_WEB_HISTORY_INTERVAL, value);
        return SUCCESS;

    } else {
        return FAIL;
    }
}
static int updateSummaryInterval(char* value){
    if (isNumericOk(value, SUMMARY_INTERVAL_MIN, SUMMARY_INTERVAL_MAX)) {
        setConfigTextValue(CONFIG_WEB_SUMMARY_INTERVAL, value);
        return SUCCESS;

    } else {
        return FAIL;
    }
}

static int updateColour(char* name, char* value){
    if (isColourOk(value)) {
        char* filterName = name + strlen(CONFIG_WEB_COLOUR) + 1;
        if (isFilterNameOk(filterName)) {
            char colTxt[7];
            sprintf(colTxt, "#%s", value);
            
            setConfigTextValue(name, colTxt);
            return SUCCESS;
            
        } else {
            return FAIL;
        }

    } else {
        return FAIL;
    }
}

static int updateRssItems(char* value){
    if (isNumericOk(value, RSS_ITEMS_MIN, RSS_ITEMS_MAX)) {
        setConfigTextValue(CONFIG_WEB_RSS_ITEMS, value);
        return SUCCESS;

    } else {
        return FAIL;
    }
}

static int updateRssFreq(char* value){
    if (isNumericOk(value, RSS_FREQ_MIN, RSS_FREQ_MAX)) {
        setConfigTextValue(CONFIG_WEB_RSS_FREQ, value);
        return SUCCESS;

    } else {
        return FAIL;
    }
}

static int updateRssHostName(char* value){
    if (isValueLenOk(value, RSS_HOST_NAME_MAX_LEN) && noDodgyChars(value)) {
        setConfigTextValue(CONFIG_WEB_RSS_HOST, value);
        return SUCCESS;

    } else {
        return FAIL;
    }
}

static int updateWebServerName(char* value){
    if (isValueLenOk(value, WEB_SERVER_NAME_MAX_LEN) && noDodgyChars(value)) {
        setConfigTextValue(CONFIG_WEB_SERVER_NAME, value);
        return SUCCESS;

    } else {
        return FAIL;
    }
}

static void writeConfig(SOCKET fd, int allowAdmin){
 // Write the JSON object out to the stream
    WRITE_HEADERS_OK(fd, MIME_JS, TRUE);

    WRITE_TEXT(fd, "var config = { ");
    char* val = getConfigText(CONFIG_WEB_MONITOR_INTERVAL, FALSE);
    writeNumConfigValue(fd, "monitorInterval", val);
    free(val);

    WRITE_TEXT(fd, ", ");
    val = getConfigText(CONFIG_WEB_SUMMARY_INTERVAL, FALSE);
    writeNumConfigValue(fd, "summaryInterval", val);
    free(val);

    WRITE_TEXT(fd, ", ");
    val = getConfigText(CONFIG_WEB_HISTORY_INTERVAL, FALSE);
    writeNumConfigValue(fd, "historyInterval", val);
    free(val);

    WRITE_TEXT(fd, ", ");
    writeNumConfigNumValue(fd, "monitorIntervalMin", MONITOR_INTERVAL_MIN);
    WRITE_TEXT(fd, ", ");
    writeNumConfigNumValue(fd, "monitorIntervalMax", MONITOR_INTERVAL_MAX);
    WRITE_TEXT(fd, ", ");
    writeNumConfigNumValue(fd, "historyIntervalMin", HISTORY_INTERVAL_MIN);
    WRITE_TEXT(fd, ", ");
    writeNumConfigNumValue(fd, "historyIntervalMax", HISTORY_INTERVAL_MAX);
    WRITE_TEXT(fd, ", ");
    writeNumConfigNumValue(fd, "summaryIntervalMin", SUMMARY_INTERVAL_MIN);
    WRITE_TEXT(fd, ", ");
    writeNumConfigNumValue(fd, "summaryIntervalMax", SUMMARY_INTERVAL_MAX);

    WRITE_TEXT(fd, ", ");
    val = getConfigText(CONFIG_WEB_SERVER_NAME, FALSE);
    writeTxtConfigValue(fd, "serverName", val);
    free(val);

    WRITE_TEXT(fd, ", ");
    writeNumConfigValue(fd, "allowAdmin", allowAdmin ? "1" : "0");

    WRITE_TEXT(fd, ", ");
    writeTxtConfigValue(fd, "version", VERSION);

    WRITE_TEXT(fd, ", ");
    val = getConfigText(CONFIG_WEB_RSS_ITEMS, FALSE);
    writeNumConfigValue(fd, "rssItems", val);
    free(val);

    WRITE_TEXT(fd, ", ");
    val = getConfigText(CONFIG_WEB_RSS_FREQ, FALSE);
    writeNumConfigValue(fd, "rssFreq", val);
    free(val);

    WRITE_TEXT(fd, ", ");
    val = getConfigText(CONFIG_WEB_RSS_HOST, FALSE);
    writeTxtConfigValue(fd, "rssHost", val);
    free(val);
    
    WRITE_TEXT(fd, ", ");
    writeHostList(fd);

    WRITE_TEXT(fd, ", ");
    writeFilterList(fd);
    
    struct NameValuePair* colourConfig = getConfigPairsWithPrefix(CONFIG_WEB_COLOUR);
    struct NameValuePair* firstColourConfig = colourConfig;
    while(colourConfig != NULL){
        WRITE_TEXT(fd, ", ");
        writeTxtConfigValue(fd, colourConfig->name, colourConfig->value);
        colourConfig = colourConfig->next;
    }
    freeNameValuePairs(firstColourConfig);

    WRITE_TEXT(fd, " };");
}

static void writeNumConfigValue(SOCKET fd, char* key, char* value){
 // Helper function, writes a key/value pair to the stream
    char txt[64];
    sprintf(txt, "\"%s\" : %s", key, value);
    WRITE_TEXT(fd, txt);
}

static void writeNumConfigNumValue(SOCKET fd, char* key, int value){
 // Helper function, writes a key/value pair to the stream
    char txt[64];
    sprintf(txt, "\"%s\" : %d", key, value);
    WRITE_TEXT(fd, txt);
}

static void writeTxtConfigValue(SOCKET fd, char* key, char* value){
 // Helper function, writes a key/value pair to the stream surrounding the value with quotes
    char txt[64];
    sprintf(txt, "\"%s\" : \"%s\"", key, value);
    WRITE_TEXT(fd, txt);
}

static void writeHostList(SOCKET fd){
    sqlite3_stmt* stmt = getStmt(HOST_SQL);
    int rc;

    WRITE_TEXT(fd, "\"hosts\" : [");
    
    int firstResult = TRUE;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (firstResult == FALSE){
            WRITE_TEXT(fd, ",");
        }

        WRITE_TEXT(fd, "\"");
        WRITE_TEXT(fd, sqlite3_column_text(stmt, 0));
        WRITE_TEXT(fd, "\"");

        firstResult = FALSE;
    }
    
    WRITE_TEXT(fd, "]");

    finishedStmt(stmt);
}

static void writeFilter(SOCKET fd, struct Filter* filter){
    WRITE_TEXT(fd, "{");
    writeNumConfigNumValue(fd, "id",   filter->id);
    WRITE_TEXT(fd, ", ");
    writeTxtConfigValue(fd, "name", filter->name);
    WRITE_TEXT(fd, ", ");
    writeTxtConfigValue(fd, "desc", filter->desc);
    
    WRITE_TEXT(fd, "}");
}

static void writeFilterList(SOCKET fd){
    struct Filter* filters = readFilters();
    
    WRITE_TEXT(fd, "\"filters\" : [");
    
    struct Filter* filter = filters;
    int first = TRUE;
    
    while (filter != NULL) {
        if (first == FALSE){
            WRITE_TEXT(fd, ",");
        }
        writeFilter(fd, filter);
        
        first = FALSE;
        filter = filter->next;
    }
    WRITE_TEXT(fd, "]");
    freeFilters(filters);
}