#ifdef UNIT_TESTING
	#import "test.h"
#endif
#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <unistd.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "client.h"

#define ALERT_SQL_SELECT_ALL                 "SELECT id, name, active, bound, filter, amount FROM alert;"
#define ALERT_SQL_SELECT_INTERVAL            "SELECT yr, mn, dy, wk, hr FROM interval WHERE id=?;"
#define ALERT_SQL_SELECT_INTERVALS_FOR_ALERT "SELECT interval_id FROM alert_interval WHERE alert_id=?"
#define ALERT_SQL_DELETE_ALERT               "DELETE FROM alert WHERE id=?;"
#define ALERT_SQL_DELETE_INTERVAL            "DELETE FROM interval WHERE id IN (SELECT interval_id FROM alert_interval WHERE alert_id=? UNION SELECT bound FROM alert WHERE id=?);"
#define ALERT_SQL_DELETE_ALERT_INTERVAL      "DELETE FROM alert_interval WHERE alert_id=?;"
#define ALERT_SQL_SELECT_MAX_ALERT_ID        "SELECT max(id) FROM alert;"
#define ALERT_SQL_SELECT_MAX_INTERVAL_ID     "SELECT max(id) FROM interval;"
#define ALERT_SQL_INSERT_ALERT               "INSERT INTO alert (id, name, active, bound, filter, amount) VALUES (?,?,?,?,?,?);"
#define ALERT_SQL_INSERT_INTERVAL            "INSERT INTO interval (id, yr, mn, dy, wk, hr) VALUES (?,?,?,?,?,?);"
#define ALERT_SQL_INSERT_ALERT_INTERVAL      "INSERT INTO alert_interval (alert_id, interval_id) VALUES (?,?);"
#define ALERT_SQL_SELECT_ROWS                "SELECT ts AS ts, dr AS dr, vl AS vl FROM data2 WHERE ts >=? AND fl=?;"
#define ALERT_SQL_TOTAL_BETWEEN              "SELECT SUM(vl) AS vl FROM data2 WHERE ts>? AND ts <=? AND fl=?"

static int getNextId(char* sql);
static struct Alert* alertForRow(sqlite3_stmt *stmtSelectAlerts, sqlite3_stmt *stmtSelectInterval, sqlite3_stmt *stmtSelectIntervalIdsForAlert);    
static struct DateCriteria* getIntervalForId(sqlite3_stmt *stmtSelectInterval, int id);
static int doAddAlert(struct Alert* alert, int alertId);
static int doRemoveAlert(int id);
int isDateCriteriaPartMatch(struct DateCriteriaPart* criteriaPart, int value);

static int addInterval(struct DateCriteria* interval){
 // Inserts a row into the 'interval' table, using the values from the DateCriteria struct
    
 // Convert each part of the struct into a string
    char* yTxt = dateCriteriaPartToText(interval->year);
    char* mTxt = dateCriteriaPartToText(interval->month);
    char* dTxt = dateCriteriaPartToText(interval->day);
    char* wTxt = dateCriteriaPartToText(interval->weekday);
    char* hTxt = dateCriteriaPartToText(interval->hour);
    
 // Get the next unique id
    int id = getNextId(ALERT_SQL_SELECT_MAX_INTERVAL_ID);
    
 // Run the SQL
    sqlite3_stmt *stmtInsertInterval = getStmt(ALERT_SQL_INSERT_INTERVAL);
    sqlite3_bind_int(stmtInsertInterval,  1, id);
    sqlite3_bind_text(stmtInsertInterval, 2, yTxt, strlen(yTxt), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmtInsertInterval, 3, mTxt, strlen(mTxt), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmtInsertInterval, 4, dTxt, strlen(dTxt), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmtInsertInterval, 5, wTxt, strlen(wTxt), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmtInsertInterval, 6, hTxt, strlen(hTxt), SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmtInsertInterval);

 // Tidy up    
    finishedStmt(stmtInsertInterval);
    
    free(yTxt);
    free(mTxt);
    free(dTxt);
    free(wTxt);
    free(hTxt);
    
 // Return
    if (rc != SQLITE_DONE){
        logMsg(LOG_ERR, "stmtInsertInterval failed: %d", rc);
        return ALERT_ID_FAIL;   
    } else {
        return id;
    }
}

static int addAlertInterval(int alertId, int intervalId){
 // Inserts a row into the 'alert_interval' table
    sqlite3_stmt *stmtInsertAlertInterval = getStmt(ALERT_SQL_INSERT_ALERT_INTERVAL);
    sqlite3_bind_int(stmtInsertAlertInterval, 1, alertId);
    sqlite3_bind_int(stmtInsertAlertInterval, 2, intervalId);
    int rc = sqlite3_step(stmtInsertAlertInterval);
    
 // Tidy up
    finishedStmt(stmtInsertAlertInterval);
    
 // Return
    if (rc != SQLITE_DONE){
        logMsg(LOG_ERR, "stmtInsertAlertInterval failed: %d", rc);
        return FAIL;   
    } else {
        return SUCCESS;
    }
}

struct Data* getTotalsForAlert(struct Alert* alert, time_t now){
 // Calculate how much data has been transferred during the interval specified by the Alert

 // Find the most recent time that matches the Alert 'bound' criteria and that is also before 'now'
    time_t ts = findFirstMatchingDate(alert->bound, now);
    
    struct Data* totals;
    if (ts >= 0){
        if (alert->periods == NULL){
         /* Special case - the Alert covers all days/times since its beginning. The totals will
            therefore be everything matching the filter with a ts greater 
            than the 'first matching' ts we found above, and less than 'now'. */
            sqlite3_stmt *stmt = getStmt(ALERT_SQL_TOTAL_BETWEEN);
            sqlite3_bind_int(stmt, 1, ts);
            sqlite3_bind_int(stmt, 2, now);
            sqlite3_bind_int(stmt, 3, alert->filter);
            totals = runSelect(stmt);
			finishedStmt(stmt);            
			
            if (totals == NULL){
            	totals = allocData();	
            }
            
        } else {
         /* This alert covers certains times/days only - we retrieve all rows from the database that might
         	match (ie all those later than the 'first matching' ts we found above) and then check each one 
         	in turn against each of the periods for which this alert is active. If a given row matches any 
         	of the periods, then its values are added to the total and we continue with the next row. */
        	totals = allocData();
        	
         // Get all rows from the db after the first matching ts
            sqlite3_stmt *stmt = getStmt(ALERT_SQL_SELECT_ROWS);
            sqlite3_bind_int(stmt, 1, ts + 1);
            sqlite3_bind_int(stmt, 2, alert->filter);
            
            struct Data* firstRow = runSelect(stmt);
            struct Data* resultRow = firstRow;
            struct DateCriteria* period;
            
         // Check each db row in turn
            while(resultRow != NULL){
            	if (resultRow->ts < now){
	                period = alert->periods;
	             // Check each of the alert's periods to see if it matches the ts of the current row
	                while(period != NULL){
	                    if (isDateCriteriaMatch(period, resultRow->ts - resultRow->dr)){
	                     // There is a match - add the rows values to the totals and move on to the next row
                            totals->vl += resultRow->vl;
	                        break;    
	                    }
	                    period = period->next;   
	                }
	            }
                resultRow = resultRow->next;   
            }
            
         // Tidy up
            freeData(firstRow);
            finishedStmt(stmt);
        }
    } else {
     // No matching time was found (the Alert starts in the future maybe?)
    	totals = allocData();
    }

    return totals;
}

int updateAlert(struct Alert* alert) {
 /* Update the database values for an existing alert, since we do this in 2 parts (a delete and
 	then an insert) we use a database transaction. */
    beginTrans(FALSE);
    
    int alertId = alert->id;
    int status;
    
 // Delete the existing alert
    status = doRemoveAlert(alertId);
    if (status == SUCCESS){
     // and re-add it with its new values
        status = doAddAlert(alert, alertId);    
    }
    
    if (status == SUCCESS){
        commitTrans();
    } else {
        rollbackTrans();
    }
    
    return status;
}
int addAlert(struct Alert* alert) {
 // Create a new alert
    beginTrans(FALSE);
    
 // Find the next available unique id
    int alertId = getNextId(ALERT_SQL_SELECT_MAX_ALERT_ID);
    int status = doAddAlert(alert, alertId);
    
    if (status == SUCCESS){
        commitTrans();
        return alertId;
    } else {
        rollbackTrans();
        return ALERT_ID_FAIL;
    }
}
static int doAddAlert(struct Alert* alert, int alertId){
 // Perform the various table inserts required to set up a new alert
    int status = SUCCESS;
    
 /* Insert an entry into the 'interval' table for the alerts 'bound' (ie when it starts) and
 	remember the id - we'll need it shortly*/
    int boundId = addInterval(alert->bound);
    if (boundId == ALERT_ID_FAIL){
        status = FAIL;
    }
    
    if (status == SUCCESS){
 	 // Add an entry into the main 'alert' table    	
        sqlite3_stmt *stmtInsertAlert = getStmt(ALERT_SQL_INSERT_ALERT);
        sqlite3_bind_int(stmtInsertAlert,   1, alertId);
        sqlite3_bind_text(stmtInsertAlert,  2, alert->name, strlen(alert->name), SQLITE_TRANSIENT);
        sqlite3_bind_int(stmtInsertAlert,   3, alert->active);
        sqlite3_bind_int(stmtInsertAlert,   4, boundId); // this is the unique id we remembered previously
        sqlite3_bind_int(stmtInsertAlert,   5, alert->filter);
        sqlite3_bind_int64(stmtInsertAlert, 6, alert->amount);
        
        int rc = sqlite3_step(stmtInsertAlert);
        if (rc != SQLITE_DONE){
        	logMsg(LOG_ERR, "stmtInsertAlert failed: %d", rc);
            status = FAIL;   
        }
        finishedStmt(stmtInsertAlert);
    }
    
    if (status == SUCCESS){
     // For each of the alerts 'periods' (ie the times when it is active) we must create 2 rows...
        struct DateCriteria* period = alert->periods;
        int periodId, rc;
        while (period != NULL){
         // Add an entry into the 'interval' table for this period
            periodId = addInterval(period);
            if (periodId == ALERT_ID_FAIL){
                status = FAIL;
                break;
            }
            
         // Add an entry into the 'alert_interval' table to associate the period with the alert
            rc = addAlertInterval(alertId, periodId);
            if (rc == FAIL){
                status = FAIL;
                break;    
            }
            period = period->next;
        }
    }
    
    return status;
}

struct Alert* getAlerts(){
 // Return a list of all the alerts that are defined in the database
    sqlite3_stmt *stmtSelectAlerts              = getStmt(ALERT_SQL_SELECT_ALL);
    sqlite3_stmt *stmtSelectInterval            = getStmt(ALERT_SQL_SELECT_INTERVAL);
    sqlite3_stmt *stmtSelectIntervalIdsForAlert = getStmt(ALERT_SQL_SELECT_INTERVALS_FOR_ALERT);
	struct Alert* result = NULL;
	struct Alert* thisAlert = NULL;

	int rc;

    beginTrans(FALSE);
 // Loop once for each entry in the 'alert' table
	while ((rc = sqlite3_step(stmtSelectAlerts)) == SQLITE_ROW){
		thisAlert = alertForRow(stmtSelectAlerts, stmtSelectInterval, stmtSelectIntervalIdsForAlert);
		appendAlert(&result, thisAlert);
	}
    commitTrans();

 // Tidy up
    finishedStmt(stmtSelectIntervalIdsForAlert);
    finishedStmt(stmtSelectInterval);
    finishedStmt(stmtSelectAlerts);

	if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, "stmtSelectAlerts failed: %d.", rc);
	}

	return result;    
}
int removeAlert(int id){
 // Delete the alert with the specified 'id' (as well as all its intervals)
    beginTrans(FALSE);
    
    int status = doRemoveAlert(id);
    
    if (status == SUCCESS){
        commitTrans();    
    } else {
        rollbackTrans();
    }

    return status;
}
static int doRemoveAlert(int id){
 // Run the SQL to remove the specified alert, and all its assocaited interval/alert_interval rows
    int rc;
    int status;

    sqlite3_stmt *stmtDeleteAlert         = getStmt(ALERT_SQL_DELETE_ALERT);
    sqlite3_stmt *stmtDeleteInterval      = getStmt(ALERT_SQL_DELETE_INTERVAL);
    sqlite3_stmt *stmtDeleteIntervalAlert = getStmt(ALERT_SQL_DELETE_ALERT_INTERVAL);
    
 // Remove rows from the 'interval' table
    sqlite3_bind_int(stmtDeleteInterval, 1, id);
    sqlite3_bind_int(stmtDeleteInterval, 2, id);
    rc = sqlite3_step(stmtDeleteInterval);
    if (rc == SQLITE_DONE){
    	status = SUCCESS;	
    } else {
    	logMsg(LOG_ERR, "stmtDeleteInterval failed: %d", rc);
    	status = FAIL;
    }
    
    if (status == SUCCESS){
     // Remove row from the alert table
    	sqlite3_bind_int(stmtDeleteAlert, 1, id);
    	rc = sqlite3_step(stmtDeleteAlert);
    	if (rc == SQLITE_DONE){
	    	status = SUCCESS;	
	    } else {
	    	logMsg(LOG_ERR, "stmtDeleteAlert failed: %d", rc);
	    	status = FAIL;
	    }
    }
    
    if (status == SUCCESS){
     // Remove rows from the alert_interval table
        sqlite3_bind_int(stmtDeleteIntervalAlert, 1, id);
        rc = sqlite3_step(stmtDeleteIntervalAlert);
        if (rc == SQLITE_DONE){
	    	status = SUCCESS;	
	    } else {
	    	logMsg(LOG_ERR, "stmtDeleteIntervalAlert failed: %d", rc);
	    	status = FAIL;
	    }
    }
    
 // Tidy up
    finishedStmt(stmtDeleteIntervalAlert);
    finishedStmt(stmtDeleteInterval);
    finishedStmt(stmtDeleteAlert);
    
    return status;
}

static struct Alert* alertForRow(sqlite3_stmt *stmtSelectAlerts, 
        sqlite3_stmt *stmtSelectInterval, sqlite3_stmt *stmtSelectIntervalIdsForAlert){
 // Create a new Alert struct for the db row currently pointed to by the 'stmtSelectAlerts' statement
    struct Alert* alert = allocAlert();
    
 // Populate the Alert struct by reading row values from stmtSelectAlerts
    alert->id = sqlite3_column_int(stmtSelectAlerts, 0);
    
    const unsigned char* name = sqlite3_column_text(stmtSelectAlerts, 1);
    setAlertName(alert, name);

    alert->active  = sqlite3_column_int(stmtSelectAlerts, 2);
    
    int boundId = sqlite3_column_int(stmtSelectAlerts, 3);
 // Create the DateCriteria struct for the alerts 'bound' (ie. when it starts)
    alert->bound = getIntervalForId(stmtSelectInterval, boundId);
    
    alert->filter = sqlite3_column_int(stmtSelectAlerts, 4);
    alert->amount = sqlite3_column_int64(stmtSelectAlerts, 5);
    
 // Create DateCriteria structs for each of the periods of the alert (ie the times/days when it is active)
    int periodId;
    struct DateCriteria* period;
    sqlite3_bind_int(stmtSelectIntervalIdsForAlert, 1, alert->id);
    int rc;
    while((rc = sqlite3_step(stmtSelectIntervalIdsForAlert)) == SQLITE_ROW){
     // We need the id of each interval associated with this alert via the alert_interval table
        periodId = sqlite3_column_int(stmtSelectIntervalIdsForAlert, 0);
        period = getIntervalForId(stmtSelectInterval, periodId);
        
        appendDateCriteria(&(alert->periods), period);
    }
    sqlite3_reset(stmtSelectIntervalIdsForAlert);
    
    if (rc != SQLITE_DONE){
		logMsg(LOG_ERR, "stmtSelectIntervalIdsForAlert failed: %d.", rc);
	}
	
    return alert;
}

static struct DateCriteria* getIntervalForId(sqlite3_stmt *stmtSelectInterval, int id){
 // Create a DateCriteria struct populated from the db row with the specified id
    sqlite3_bind_int(stmtSelectInterval, 1, id);
    int rc = sqlite3_step(stmtSelectInterval);
    
    char* yearTxt;
    char* monthTxt;
    char* dayTxt;
    char* weekdayTxt;
    char* hourTxt;
    
    struct DateCriteria* result = NULL;
    
    if (rc == SQLITE_ROW) {
     // We found the requested interval, read out the values
        yearTxt    = sqlite3_column_text(stmtSelectInterval, 0);
        monthTxt   = sqlite3_column_text(stmtSelectInterval, 1);
        dayTxt     = sqlite3_column_text(stmtSelectInterval, 2);
        weekdayTxt = sqlite3_column_text(stmtSelectInterval, 3);
        hourTxt    = sqlite3_column_text(stmtSelectInterval, 4);
        
        result = makeDateCriteria(yearTxt, monthTxt, dayTxt, weekdayTxt, hourTxt);  
        
    } else if (rc != SQLITE_DONE) {
		logMsg(LOG_ERR, "stmtSelectInterval failed: %d", rc);
    }
    
    sqlite3_reset(stmtSelectInterval);
    
    return result;
}

struct DateCriteriaPart* getNonRelativeValue(int value){
 // Make a new DateCriteriaPart struct using the specified value, and with isRelative set to false
    struct DateCriteriaPart* part = malloc(sizeof(struct DateCriteriaPart));
    part->val1 = part->val2 = value;
    part->isRelative = FALSE;
    part->next = NULL;
    return part;
}

static int getYear(struct tm* t){
 // Return the full year value from the struct
    return t->tm_year + 1900;
}
static void setYear(struct tm* t, int year){
 // Set the year value in the struct
    t->tm_year = (year - 1900);
    normaliseTm(t);
}
static int getMonth(struct tm* t){
 // Return the 1-based month value from the struct
    return t->tm_mon + 1;
}
static void setMonth(struct tm* t, int month){
 // Set the month value in the struct
    t->tm_mon = (month - 1);
    normaliseTm(t);
}
static int getWeekday(struct tm* t){
 // Get the weekday value from the struct (Sunday = 0)
    return t->tm_wday;
}
static int getDay(struct tm* t){
 // Get the day of the month from the struct
    return t->tm_mday;
}
static void setDay(struct tm* t, int day){
 // Set the day of the month in the struct
    t->tm_mday = day;
    normaliseTm(t);
}
static int getHour(struct tm* t){
 // Get the hour of the day from the struct
    return t->tm_hour;
}
static void setHour(struct tm* t, int hour){
 // Set the hour of the day in the struct	
    t->tm_hour = hour;
    normaliseTm(t);
}

static void setDateCriteriaPart(struct DateCriteriaPart** part, struct DateCriteriaPart* newValue){
 // Change the 'part' pointer to reference the newValue argument, after freeing anything that it was previously referencing
	if (*part != NULL){
		freeDateCriteriaPart(*part);
	}
	*part = newValue;
}

/* Change the criteria struct by replacing any relative values with non-relateive values based 
   on the specified timestamp value. For example if the criteria contains *,*,*,*,-1 this is a 
   relative value meaning '1 hour ago', so we populate the criteria with values generated from 
   the ts argument, and then substract 1 hour. Before we make these changes we check that the 
   criteria's relative values make sense by applying the following rules:
     - No relative value should have a non-relative value in a higher field, eg "2010,*,*,*,-1" 
       is invalid because the hour value is relative, but we have a non-relative value in the 
       year field. The weekday value is ignored when applying this rule.
     - No relative value should have a null value in a lower field, eg "*,*,*,-1,*" is invalid 
       because the day field is relative but the hour field is null. The weekday value is ignored 
       when applying this rule.
     - There should never be more than 1 relative value in a given struct, eg "*,*,-1,*,-1" is 
       invalid because both the day and hour values are relative.
*/
int replaceRelativeValues(struct DateCriteria* criteria, time_t ts){
    int valuesOk = TRUE;
    
    int nullYear    = (criteria->year    == NULL);
    int nullMonth   = (criteria->month   == NULL);
    int nullWeekday = (criteria->weekday == NULL);
    int nullDay     = (criteria->day     == NULL);
    int nullHour    = (criteria->hour    == NULL);
    
    int relativeYear    = (!nullYear    && criteria->year->isRelative);
    int relativeMonth   = (!nullMonth   && criteria->month->isRelative);
    int relativeWeekday = (!nullWeekday && criteria->weekday->isRelative);
    int relativeDay     = (!nullDay     && criteria->day->isRelative);
    int relativeHour    = (!nullHour    && criteria->hour->isRelative);
    
    int hasRelativeValues = (relativeYear || relativeMonth || relativeWeekday || relativeDay || relativeHour);
    
    if (hasRelativeValues){
        int relativeValuesOk = TRUE;
        
     // Relative weekday doesn't make sense
        if (relativeWeekday){            
            relativeValuesOk = FALSE;
        }
        
     // No relative value should have an absolute value in a higher field, or a null value in a lower field
        if (relativeValuesOk && relativeHour){
            relativeValuesOk = (nullYear || relativeYear) && (nullMonth || relativeMonth) && (nullDay || relativeDay);
        }
        if (relativeValuesOk && relativeDay){
            relativeValuesOk = (nullYear || relativeYear) && (nullMonth || relativeMonth) && (!nullHour);
        }
        if (relativeValuesOk && relativeMonth){
            relativeValuesOk = (nullYear || relativeYear) && (!nullDay) && (!nullHour);
        }
        if (relativeValuesOk && relativeYear){
            relativeValuesOk = (!nullMonth) && (!nullDay) && (!nullHour);
        }
        
        if (relativeValuesOk){
         // We can have at most 1 non-zero relative value
            int nonZeroRelativeCount = 0;
            nonZeroRelativeCount += (relativeYear  && criteria->year->val1  > 0);
            nonZeroRelativeCount += (relativeMonth && criteria->month->val1 > 0);
            nonZeroRelativeCount += (relativeDay   && criteria->day->val1   > 0);
            nonZeroRelativeCount += (relativeHour  && criteria->hour->val1  > 0);
            
            if (nonZeroRelativeCount > 1){
                relativeValuesOk = FALSE;
            }
        }
        
        if (relativeValuesOk){      
         // We are satisfied that the struct's relative values are sensible, so we carry on
            struct tm* t = localtime((time_t *) &ts);
            int relativeValue, absoluteValue;
            
            if (relativeYear){
             // Adjust the year value, we can ignore all the other values because they must be null
                relativeValue = criteria->year->val1;
                setDateCriteriaPart(&criteria->year, getNonRelativeValue(getYear(t) - relativeValue)); 
            }
            
            if (relativeMonth){
             // Calculate the new month value
                relativeValue = criteria->month->val1;
                absoluteValue = getMonth(t) - relativeValue;
                if (absoluteValue < 1){
                 /* The month value we calculated is either 0 or negative, so we are moving back
                 	to a previous year - use 'setMonth' to normalise the month and year values and
                 	then assign them to the struct */
                    setMonth(t, getMonth(t) - relativeValue);
                    setDateCriteriaPart(&criteria->month, getNonRelativeValue(getMonth(t)));
                    setDateCriteriaPart(&criteria->year,  getNonRelativeValue(getYear(t)));
                } else {
                    setDateCriteriaPart(&criteria->month, getNonRelativeValue(absoluteValue));
                }
            }
            
            if (relativeDay){
             // Calculate the new day value
                relativeValue = criteria->day->val1;
                absoluteValue = getDay(t) - relativeValue;
                if (absoluteValue < 1){
                 /* The day value we calculated is either 0 or negative, so we are moving back
                 	to a previous month - use 'setDay' to normalise the day, month and year values 
                 	and then assign them to the struct */
                    setDay(t, getDay(t) - relativeValue);
                    setDateCriteriaPart(&criteria->day,   getNonRelativeValue(getDay(t)));
                    setDateCriteriaPart(&criteria->month, getNonRelativeValue(getMonth(t)));
                    setDateCriteriaPart(&criteria->year,  getNonRelativeValue(getYear(t)));
                } else {
                    setDateCriteriaPart(&criteria->day, getNonRelativeValue(absoluteValue));    
                }
            }
            
            if (relativeHour){
             // Calculate the hour value
                relativeValue = criteria->hour->val1;
                absoluteValue = getHour(t) - relativeValue;
               	if (absoluteValue < 0){
                 /* The hour value we calculated is negative, so we are moving back to a previous day 
                 	- use 'setHour' to normalise the hour, day, month and year values and then assign 
                 	them to the struct */
                    setHour(t, getHour(t) - relativeValue);
                    setDateCriteriaPart(&criteria->hour,  getNonRelativeValue(getHour(t)));
                    setDateCriteriaPart(&criteria->day,   getNonRelativeValue(getDay(t)));
                    setDateCriteriaPart(&criteria->month, getNonRelativeValue(getMonth(t)));
                    setDateCriteriaPart(&criteria->year,  getNonRelativeValue(getYear(t)));
                } else {
                    setDateCriteriaPart(&criteria->hour, getNonRelativeValue(absoluteValue));    
                }
            }
        } else {
        	logMsg(LOG_ERR, "replaceRelativeValues failed");
            valuesOk = FALSE;   
        }
    }
    return valuesOk;
}
int findLowestMatch(struct DateCriteriaPart* part){
 // Return the lowest value that will match the DateCriteriaPart
	if (part == NULL){
		logMsg(LOG_ERR, "findLowestMatch called with NULL argument");
	}
    int lowestMatch = part->val1;
    while (part != NULL) {
        if (part->val1 < lowestMatch){
            lowestMatch = part->val1;
        }
        part = part->next;
    }
    return lowestMatch;
}
int findHighestMatch(struct DateCriteriaPart* part){
 // Return the highest value that will match the DateCriteriaPart	
	if (part == NULL){
		logMsg(LOG_ERR, "findHighestMatch called with NULL argument");
	}
    int highestMatch = part->val2;
    
    while (part != NULL) {
        if (part->val2 > highestMatch){
            highestMatch = part->val2;
        }
        part = part->next;
    }
    
    return highestMatch;
}
int findHighestMatchAtOrBelowLimit(struct DateCriteriaPart* part, int limit){
 // Return the highest value that will match the DateCriteriaPart without exceeding the 'limit' value
    int highestMatch = -1, candidateMatch;
    
    while (part != NULL) {
     // Skip this part completely if even the lower of its values is above the limit
        if (part->val1 <= limit){
            if (part->val2 <= limit){
                candidateMatch = part->val2;
            } else {
             /* The upper value of this part is > limit, and the lower part is < limit, so we have found
                the highest possible match. */
                highestMatch = limit;
                break;
            }
            
            if (candidateMatch > highestMatch){
             // We have a new possible winner...
                highestMatch = candidateMatch;
                if (highestMatch == limit){
                    break;
                }
            } 
        }
        part = part->next;
    }
    return highestMatch;
}

time_t findFirstMatchingDate(struct DateCriteria* criteria, time_t now){
 // Find the earliest date that matches the criteria, ignorng any dates later than 'now'
	struct tm* tmCandidate = localtime(&now);
	tmCandidate->tm_min = 0;
	tmCandidate->tm_sec = 0;

 // Convert any relative parts of the criteria into concrete values, using the current date/time
    int valuesOk = replaceRelativeValues(criteria, now);
    if (!valuesOk){
        return -1;
    }
	
    int yearOk, monthOk, weekdayOk, dayOk, hourOk;
    yearOk = monthOk = weekdayOk = dayOk = hourOk = FALSE;
    
 // Keep looping round until we are satisfied with all 4 date components
    while (!(yearOk && monthOk && dayOk && hourOk)) {
        if (!yearOk){
         // Find the lowest matching year value
        	if (criteria->year != NULL){
        	 // Find the lowest year that matches the criteria
    	        int lowestMatchingYear = findLowestMatch(criteria->year);
        		if (getYear(tmCandidate) < lowestMatchingYear){
        		 /* The lowest matching year is too high for out current candidate, so stop and return NULL
        		    indicating that no match could be found (eg now='01/01/2010', criteria->year='2011-2012') */
        		    tmCandidate = NULL;
        		    break;
        		} else {
        		 // Find the highest year that matches the criteria but that is <= the candidate date's year
        		    int highestMatchingYear = findHighestMatchAtOrBelowLimit(criteria->year, getYear(tmCandidate));
        		    if (highestMatchingYear == getYear(tmCandidate)){
        		        // The candidate's year is already the highest match, so leave it alone
        		    } else {
        		     /* The candidate year must be adjusted (reduced) so that it equals the highest 
        		     	matching year of the criteria. We also need to change the month, day and hour
        		     	values of the candidate so that the candidate now matches the very end of the
        		     	new year. eg if the candidate was '10/05/2010' and the highestMatchingYear='2009'
        		     	then we change the candidate to 31/12/2009 23:00' */
        		        setDay(tmCandidate, 1);
        		        setMonth(tmCandidate, 1);
        		        setYear(tmCandidate, highestMatchingYear + 1);
        		        setHour(tmCandidate, -1);// last hour of previous year
        		        
        		     /* We now need to set all these flags back to false because we just changed the month,
        		     	day and hour values and we have no idea if they still satisfy the criteria. */
        		        monthOk = weekdayOk = dayOk = hourOk = FALSE;
        		    }
        		    
        		}
        	}
        	yearOk = TRUE;
        }

        if (!monthOk){
        	if (criteria->month != NULL){
        	 // Find the lowest month that matches the criteria
    	        int lowestMatchingMonth = findLowestMatch(criteria->month);
        		if (getMonth(tmCandidate) < lowestMatchingMonth){
        		 /* The lowest matching month of the current year is too high for out current candidate, reduce the
        		 	year by 1, and set the month to the highest one that matches. Also adjust the day and hour value 
        		 	so that the candidate remains as high as possible. */
        		    setYear(tmCandidate, getYear(tmCandidate) - 1);
        		    setDay(tmCandidate, 1);
        		    setMonth(tmCandidate, findHighestMatch(criteria->month) + 1);
        		    setHour(tmCandidate, -1);
        		    
        		 /* These flags all get set to false because the corresponding values were changed and may no longer  
        		 	satisfy the criteria. */
        		    yearOk = weekdayOk = dayOk = hourOk = FALSE;

        		} else {
        		 // Find the highest month that matches the criteria but that is <= the candidate date's month
        		    int highestMatchingMonth = findHighestMatchAtOrBelowLimit(criteria->month, getMonth(tmCandidate));
        		    if (highestMatchingMonth == getMonth(tmCandidate)){
        		     // The candidate's month is already the highest match, so leave it alone
        		    } else {
        		     /* The candidate month must be adjusted (reduced) so that it equals the highest 
        		     	matching month of the criteria. We also need to change the day and hour
        		     	values of the candidate so that the candidate now matches the very end of the
        		     	new month. */
        		        setDay(tmCandidate, 1);
        		        setMonth(tmCandidate, highestMatchingMonth + 1);
            		    setHour(tmCandidate, -1);
            		    
	        		 /* These flags all get set to false because the corresponding values were changed and may no longer  
	        		 	satisfy the criteria. */
            		    yearOk = weekdayOk = dayOk = hourOk = FALSE;
        		    }
        		}
        	}
        	monthOk = TRUE;
        }

        if (!weekdayOk){
        	if (criteria->weekday != NULL){
        	 // Find the lowest weekday that matches the criteria
    	        int lowestMatchingWeekday = findLowestMatch(criteria->weekday);
        		if (getWeekday(tmCandidate) < lowestMatchingWeekday){
        		 /* The lowest matching weekday is too high for out current candidate, reduce the day value (as little as 
        		 	possible) so that the weekday is correct and set the hour to the highest allowable value for that day. */
        		    int highestMatchingWeekday = findHighestMatch(criteria->weekday);
        		    
        		    setDay(tmCandidate, getDay(tmCandidate) - 7 + highestMatchingWeekday - getWeekday(tmCandidate));
        		    setHour(tmCandidate, 23);
        		    
        		 /* These flags all get set to false because the adjustment made to the day value may have altered one or
        		 	all of them, and so they may no longer match the criteria. */
        		    yearOk = monthOk = dayOk = hourOk = FALSE;
        		    
        		} else {
        		 // Find the highest weekday that matches the criteria but that is <= the candidate date's weekday
        		    int highestMatchingWeekday = findHighestMatchAtOrBelowLimit(criteria->weekday, getWeekday(tmCandidate));
        		    if (highestMatchingWeekday == getWeekday(tmCandidate)){
        		     // The candidate's weekday is already the highest match, so leave it alone
        		    } else {
        		     /* The candidate day must be adjusted (reduced) so that its weekday value equals the highest 
        		     	matching weekday of the criteria. We also need to change the hour value of the candidate 
        		     	so that it now matches the very end of the day in question. */
            		    setDay(tmCandidate, getDay(tmCandidate) - (getWeekday(tmCandidate) - highestMatchingWeekday));
            		    setHour(tmCandidate, 23);
            		    
		    		 /* These flags all get set to false because the adjustment made to the day value may have altered one or
		    		 	all of them, and so they may no longer match the criteria. */
            		    yearOk = monthOk = dayOk = hourOk = FALSE;
        		    }
        		}
        	}
        	weekdayOk = TRUE;
        }

        if (!dayOk){
        	if (criteria->day != NULL){
        	 // Find the lowest day that matches the criteria
    	        int lowestMatchingDay = findLowestMatch(criteria->day);
        		if (getDay(tmCandidate) < lowestMatchingDay){
        		 /* The lowest matching day is too high for out current candidate, reduce the month by 1 and set
        		 	the day to be the highest one that matches the criteria. */
        		    setDay(tmCandidate, findHighestMatch(criteria->day) + 1);
        		    setMonth(tmCandidate, getMonth(tmCandidate) - 1);
        		    setHour(tmCandidate, -1);
        		    
        		 /* These flags all get set to false because the adjustment made to the day value may have altered one or
        		 	all of them, and so they may no longer match the criteria. */
        		    yearOk = monthOk = weekdayOk = hourOk = FALSE;
        		    
        		} else {
        		 // Find the highest day that matches the criteria but that is <= the candidate date's day
        		    int highestMatchingDay = findHighestMatchAtOrBelowLimit(criteria->day, getDay(tmCandidate));
        		    if (highestMatchingDay == getDay(tmCandidate)){
					 // The candidate's day is already the highest match, so leave it alone
        		    } else {
        		     /* The candidate day must be adjusted (reduced) so that it equals the highest matching day 
        		     	of the criteria. We also need to change the hour value so that it matches the very end 
        		     	of the day in question. */
        		        setDay(tmCandidate, highestMatchingDay + 1);
        		        setHour(tmCandidate, -1);
        		        
	        		 /* These flags all get set to false because the adjustment made to the day value may have altered one or
	        		 	all of them, and so they may no longer match the criteria. */
            		    yearOk = monthOk = weekdayOk = hourOk = FALSE;
        		    }
        		}
        	}
        	dayOk = TRUE;
        }
        
        if (!hourOk){
        	if (criteria->hour != NULL){
        	 // Find the lowest hour that matches the criteria
    	        int lowestMatchingHour = findLowestMatch(criteria->hour);
        		if (getHour(tmCandidate) < lowestMatchingHour){
        		 /* The lowest matching hour is too high for out current candidate, reduce the day by 1 and set
        		 	the hour to be the highest one that matches the criteria. */
        		    setDay(tmCandidate, getDay(tmCandidate) - 1);
        		    setHour(tmCandidate, findHighestMatch(criteria->hour));        		    
        		    
        		 /* These flags all get set to false because the adjustment made to the day value may have altered one or
        		 	all of them, and so they may no longer match the criteria. */
        		    yearOk = monthOk = weekdayOk = dayOk = FALSE;
        		    
        		} else {
        		 // Find the highest hour that matches the criteria but that is <= the candidate date's hour
        		    int highestMatchingHour = findHighestMatchAtOrBelowLimit(criteria->hour, getHour(tmCandidate));
      		        setHour(tmCandidate, highestMatchingHour);
        		}
        	}
        	hourOk = TRUE;
        }
    }

	if (tmCandidate == NULL){
	 // No match could be found
	    return -1;
	} else {
	    return mktime(tmCandidate);
	}
}

int isDateCriteriaMatch(struct DateCriteria* criteria, time_t ts){
 // Check if the specified date, when evaluated as a local time, matches the criteria
	int result = FALSE;
	struct tm* dt = localtime((time_t *) &ts);
	while(criteria != NULL){
		result = isDateCriteriaPartMatch(criteria->year, getYear(dt)) && 
			isDateCriteriaPartMatch(criteria->month, getMonth(dt)) && 
			isDateCriteriaPartMatch(criteria->day, getDay(dt)) && 
			isDateCriteriaPartMatch(criteria->weekday, getWeekday(dt)) && 
			isDateCriteriaPartMatch(criteria->hour, getHour(dt));
		if (result == TRUE){
		 // We found a DateCriteria that matched the date/time, so stop and return TRUE
			break;
		}
		criteria = criteria->next;
	}
	return result;
}

int isDateCriteriaPartMatch(struct DateCriteriaPart* criteriaPart, int value){
 // Check if the specified value (which represents a date component), matches the criteria part
	if (criteriaPart == NULL){
		return TRUE; // it was a '*'
	} else {
		int foundMatch = FALSE;
		while(criteriaPart != NULL){
			if (criteriaPart->isRelative == TRUE){
				//TODO error
			}
			if (criteriaPart->val1 <= value && criteriaPart->val2 >= value){
				foundMatch = TRUE;
				break;
			}
			criteriaPart = criteriaPart->next;
		}
		return foundMatch;
	}
}
