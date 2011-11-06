#define _GNU_SOURCE
#include <string.h>
#ifdef _WIN32
    #define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include "client.h"
#include "bmws.h"
#include "common.h"

/*
Handles '/alert' requests received by the web server.
*/
#define MAX_PART_LENGTH 256
#define BAD_NUM -1
#define DELIMS "' []"

void processAlertList(SOCKET fd);
void processAlertDelete(SOCKET fd, struct NameValuePair* params);
void processAlertUpdate(SOCKET fd, struct NameValuePair* params);
void processAlertStatus(SOCKET fd, struct NameValuePair* params);
void processAlertCreate(SOCKET fd, struct NameValuePair* params);

void processAlertRequest(SOCKET fd, struct Request* req, int allowAdmin){
    struct NameValuePair* params = req->params;

    char* action = getValueForName("action", params, "");

    if (strcmp("list", action) == 0) {
        processAlertList(fd);

    } else if (strcmp("delete", action) == 0) {
     // Deleting alerts is only allowed with admin access
        if (allowAdmin){
            processAlertDelete(fd, params);
        } else {
            WRITE_HEADERS_FORBIDDEN(fd, "alert delete");
        }

    } else if (strcmp("update", action) == 0) {
     // Updating alerts is only allowed with admin access
        if (allowAdmin){
            processAlertUpdate(fd, params);
        } else {
            WRITE_HEADERS_FORBIDDEN(fd, "alert update");
        }

    } else if (strcmp("status", action) == 0) {
        processAlertStatus(fd, params);

    } else if (strcmp("create", action) == 0) {
     // Creating alerts is only allowed with admin access
        if (allowAdmin){
            processAlertCreate(fd, params);
        } else {
            WRITE_HEADERS_FORBIDDEN(fd, "alert create");
        }

    } else {
     // Missing/invalid 'action' parameter
        WRITE_HEADERS_SERVER_ERROR(fd, "Missing/invalid 'action' parameter: '%s'", action);
    }
}

static void dateCriteriaToArray(struct DateCriteria* interval, char *arr[]){
    arr[0] = dateCriteriaPartToText(interval->year);
    arr[1] = dateCriteriaPartToText(interval->month);
    arr[2] = dateCriteriaPartToText(interval->day);
    arr[3] = dateCriteriaPartToText(interval->weekday);
    arr[4] = dateCriteriaPartToText(interval->hour);
    arr[5] = NULL;
}

static void freeArray(char* arr[]){
    int i=0;
    while(arr[i] != NULL){
        free(arr[i++]);
    }
}

static void writeAlertToJson(SOCKET fd, struct Alert* alert){
    WRITE_TEXT(fd, "{");
    WRITE_NUM_VALUE_TO_JSON(fd,  "id",     alert->id);
    WRITE_TEXT(fd, ",");
    WRITE_TEXT_VALUE_TO_JSON(fd, "name",   alert->name);
    WRITE_TEXT(fd, ",");
    WRITE_NUM_VALUE_TO_JSON(fd,  "active", alert->active);
    WRITE_TEXT(fd, ",");
    WRITE_NUM_VALUE_TO_JSON(fd,  "filter", alert->filter);
    WRITE_TEXT(fd, ",");
    WRITE_NUM_VALUE_TO_JSON(fd,  "amount", alert->amount);
    WRITE_TEXT(fd, ",");

    char *bound[6];
    dateCriteriaToArray(alert->bound, bound);
    WRITE_TEXT_ARRAY_TO_JSON(fd, "bound", bound);
    freeArray(bound);

    WRITE_TEXT(fd, ",\"periods\" : [");
    struct DateCriteria* period = alert->periods;

    int isFirstPeriod = TRUE;
    while (period != NULL){
        if (!isFirstPeriod){
            WRITE_TEXT(fd, ",");
        }

        char *periodArray[6];
        dateCriteriaToArray(period, periodArray);
        WRITE_TEXT_ARRAY_TO_JSON(fd, NULL, periodArray);
        freeArray(periodArray);

        period = period->next;
        isFirstPeriod = FALSE;
    }
    WRITE_TEXT(fd, "]}");
}

void processAlertList(SOCKET fd){
    WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);
    WRITE_TEXT(fd, "[");

    struct Alert* firstAlert = getAlerts();
    struct Alert* alert = firstAlert;

    int isFirstAlert = TRUE;

    while (alert != NULL){
        if (!isFirstAlert){
            WRITE_TEXT(fd, ",");
        }
        writeAlertToJson(fd, alert);
        alert = alert->next;
        isFirstAlert = FALSE;
    }
    WRITE_TEXT(fd, "]");
    freeAlert(firstAlert);
}

void processAlertDelete(SOCKET fd, struct NameValuePair* params){
    int id = getValueNumForName("id", params, BAD_NUM);

    if (id == BAD_NUM){
        WRITE_HEADERS_SERVER_ERROR(fd, "Invalid alert id in delete request: %s", getValueForName("id", params, NULL));
    } else {
        int status = removeAlert(id);
        if (status == SUCCESS){
            // done
            WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);
            WRITE_TEXT(fd, "{}");

        } else {
            WRITE_HEADERS_SERVER_ERROR(fd, "removeAlert() failed");
        }
    }
}

void processAlertStatus(SOCKET fd, struct NameValuePair* params){
    struct Alert* firstAlert = getAlerts();
    struct Alert* alert = firstAlert;

    struct Data* totals = NULL;

    WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);

    int first = TRUE;
    WRITE_TEXT(fd, "[");
    while (alert != NULL) {
        if (!first){
            WRITE_TEXT(fd, ",");
        }
        first = FALSE;

        totals = getTotalsForAlert(alert, getTime());
        BW_INT total = totals->vl;
        freeData(totals);

        WRITE_TEXT(fd, "{");
        WRITE_NUM_VALUE_TO_JSON(fd,  "id",      alert->id);
        WRITE_TEXT(fd, ",");
        WRITE_TEXT_VALUE_TO_JSON(fd, "name",    alert->name);
        WRITE_TEXT(fd, ",");
        WRITE_NUM_VALUE_TO_JSON(fd,  "current", total);
        WRITE_TEXT(fd, ",");
        WRITE_NUM_VALUE_TO_JSON(fd,  "limit",   alert->amount);
        WRITE_TEXT(fd, "}");

        alert = alert->next;
    }
    WRITE_TEXT(fd, "]");

    freeAlert(firstAlert);
}

static struct DateCriteria* makeSingleDateCriteriaFromTxt(char* txt){
 // txt looks like: ['*','*','1','3','4-5']
    char* txtCopy = strdupa(txt);

    char *yearTxt = strtok(txtCopy, DELIMS);
    strtok(NULL, DELIMS); // comma

    char *monthTxt = strtok(NULL, DELIMS);
    strtok(NULL, DELIMS); // comma

    char *dayTxt = strtok(NULL, DELIMS);
    strtok(NULL, DELIMS); // comma

    char *weekdayTxt = strtok(NULL, DELIMS);
    strtok(NULL, DELIMS); // comma

    char *hourTxt = strtok(NULL, DELIMS);

    assert(strtok(NULL, DELIMS) == NULL);

    return makeDateCriteria(yearTxt, monthTxt, dayTxt, weekdayTxt, hourTxt);
}

static struct DateCriteria* makeMultipleDateCriteriaFromTxt(char* txt){
 // txt looks like: [['*','*','1','3','4-5'],['*','*','1','3','4-5']]
    struct DateCriteria* result = NULL;
    char* txtCopy = strdupa(txt);
    char* tmp = strtok(txtCopy, "[]");

    while (tmp != NULL) {
        if (strcmp(tmp, ",") != 0){
            struct DateCriteria* item = makeSingleDateCriteriaFromTxt(tmp);
            appendDateCriteria(&result, item);
        }
     // Cant just pass in NULL here because we used strtok in makeSingleDateCriteriaFromTxt
        tmp = strtok(tmp + strlen(tmp) + 1, "[]");
    }

    return result;
}

void processAlertUpdate(SOCKET fd, struct NameValuePair* params){
    int    id         = getValueNumForName( "id",        params, BAD_NUM);
    char*  name       = getValueForName(    "name",      params, NULL);
    int    active     = getValueNumForName( "active",    params, BAD_NUM);
    int    fl         = getValueNumForName( "filter",    params, BAD_NUM);
    char*  amountTxt  = getValueForName(    "amount",    params, NULL);
    char*  boundTxt   = getValueForName(    "bound",     params, NULL);
    char*  periodsTxt = getValueForName(    "periods",   params, NULL);
    BW_INT amount = strToBwInt(amountTxt, BAD_NUM);

    if (id == BAD_NUM || name == NULL || active == BAD_NUM || fl == BAD_NUM || amount == BAD_NUM || boundTxt == NULL || periodsTxt == NULL) {
        WRITE_HEADERS_SERVER_ERROR(fd, "processAlertUpdate param bad/missing id=%s, name=%s, active=%s, filter=%s, amount=%s, bound=%s, periods=%s",
                getValueForName("id", params, NULL),
                name,
                getValueForName("active", params, NULL),
                getValueForName("filter", params, NULL),
                amountTxt, boundTxt, periodsTxt);

    } else {
        struct Alert* alert = allocAlert();

        alert->id        = id;
        alert->name      = strdup(name);
        alert->active    = active;
        alert->bound     = makeSingleDateCriteriaFromTxt(boundTxt);
        alert->periods   = makeMultipleDateCriteriaFromTxt(periodsTxt);
        alert->filter    = fl;
        alert->amount    = amount;

        int result = updateAlert(alert);
        freeAlert(alert);

        if (result == SUCCESS){
            WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);
            WRITE_TEXT(fd, "{}");

        } else {
            WRITE_HEADERS_SERVER_ERROR(fd, "updateAlert() failed");
        }
    }
}

void processAlertCreate(SOCKET fd, struct NameValuePair* params){
    char* name       = getValueForName(    "name",      params, NULL);
    int   active     = getValueNumForName( "active",    params, BAD_NUM);
    int   fl         = getValueNumForName( "filter",    params, BAD_NUM);
    char* amountTxt  = getValueForName(    "amount",    params, NULL);
    char* boundTxt   = getValueForName(    "bound",     params, NULL);
    char* periodsTxt = getValueForName(    "periods",   params, NULL);

    BW_INT amount = strToBwInt(amountTxt, BAD_NUM);

    if (name == NULL || active == BAD_NUM || fl == BAD_NUM || amount == BAD_NUM || boundTxt == NULL || periodsTxt == NULL) {
        WRITE_HEADERS_SERVER_ERROR(fd, "processAlertCreate param bad/missing name=%s, active=%s, filter=%s, amount=%s, bound=%s, periods=%s",
                name,
                getValueForName("active", params, NULL),
                getValueForName("filter", params, NULL),
                amountTxt, boundTxt, periodsTxt);

    } else {
        struct Alert* alert = allocAlert();

        alert->name      = strdup(name);
        alert->active    = active;
        alert->bound     = makeSingleDateCriteriaFromTxt(boundTxt);
        alert->periods   = makeMultipleDateCriteriaFromTxt(periodsTxt);
        alert->filter    = fl;
        alert->amount    = amount;
        int result = addAlert(alert);

        freeAlert(alert);

        if (result != ALERT_ID_FAIL){
            WRITE_HEADERS_OK(fd, MIME_JSON, TRUE);
            WRITE_TEXT(fd, "{}");

        } else {
            WRITE_HEADERS_SERVER_ERROR(fd, "addAlert failed");
        }
    }
}
