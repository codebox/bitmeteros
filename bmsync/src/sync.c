#ifdef _WIN32
    #include <winsock2.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"
#include "capture.h"
#include "bmsync.h"
#include "sqlite3.h"
#include "bmws.h"

#define SELECT_MAX_TS_FOR_LOCAL "SELECT MAX(ts) AS ts FROM data, filter WHERE data.fl=filter.id AND filter.host is null"
#define SELECT_MAX_TS_FOR_HOST  "SELECT MAX(ts) AS ts FROM data, filter WHERE data.fl=filter.id AND filter.host = ?"

struct Filter* parseFilterRow(char* row, char* host){
 // Parse a single row from the response into a Filter struct
    int id;
    char filterName[SMALL_BUFSIZE];
    char filterDesc[SMALL_BUFSIZE];
    char filterExpr[BUFSIZE];

 // Row format is: filter:id,name,desc,expr
    int args = sscanf(row, FILTER_ROW_PREFIX "%d,%[^,],%[^,],%s", &id, filterName, filterDesc, filterExpr);
    if (args == 4){
        return allocFilter(id, filterDesc, filterName, filterExpr, host);
    } else {
        return NULL;
    }
}

struct Data* parseDataRow(char* row){
 // Parse a single row from the response into a Data struct
    char addr[MAX_ADDR_LEN];
    
    struct Data* data = allocData();

 // Row format is:  ts,dr,vl,fl
    int args = sscanf(row, "%d,%d,%llu,%d", (int*)&(data->ts), &(data->dr), &(data->vl), &(data->fl));
    if (args == 4){
        return data;
    } else {
        freeData(data);
        return NULL;    
    }
}

time_t getMaxTsForHost(char* alias){
 // Look for rows in the local database for this particular alias, return the ts value of the newest one (or 0)
    sqlite3_stmt* stmt;
    if (alias == NULL){
        stmt = getStmt(SELECT_MAX_TS_FOR_LOCAL);
    } else {
        stmt = getStmt(SELECT_MAX_TS_FOR_HOST);
        sqlite3_bind_text(stmt, 1, alias, -1, SQLITE_TRANSIENT);
    }
    struct Data* result = runSelect(stmt);
    finishedStmt(stmt);

    time_t ts;
    if (result == NULL){
     // No rows in the table for this host yet
        ts = 0;
    } else {
     // We have seen this host before
        ts = result->ts;
        assert(result->next == NULL); 
        freeData(result);
    }

    return ts;
}

int startsWith(char* txt, char* start){
    char *ptr = strstr(txt, start);
    return ptr == txt;
}

int getLocalId(struct RemoteFilter* remoteFilter, int filterId){
    while (remoteFilter != NULL) {
        if (remoteFilter->remoteId == filterId){
            return remoteFilter->localId;
        }
        remoteFilter = remoteFilter->next;
    }
    return 0;
}

int getLocalFilter(struct Filter *remoteFilter){
    struct Filter* matchingFilter = getFilter(remoteFilter->name, remoteFilter->host);
    
    int localFilterId;
    if (matchingFilter == NULL){
        localFilterId = addFilter(remoteFilter);
    } else {
        localFilterId = matchingFilter->id;
        freeFilters(matchingFilter);
    }
    
    return localFilterId;
}

struct RemoteFilter* allocRemoteFilter(int localId, int remoteId){
    struct RemoteFilter* remoteFilter = malloc(sizeof(struct RemoteFilter));
    
    remoteFilter->localId  = localId;
    remoteFilter->remoteId = remoteId;
    remoteFilter->next = NULL;
    
    return remoteFilter;
}
void freeRemoteFilters(struct RemoteFilter* remoteFilter){
    while(remoteFilter != NULL){
        struct RemoteFilter* next = remoteFilter->next;
        free(remoteFilter);
        remoteFilter = next;
    }
}
void appendRemoteFilter(struct RemoteFilter** remoteFilters, struct RemoteFilter* newRemoteFilter){
    if (*remoteFilters == NULL){
        *remoteFilters = newRemoteFilter;
    } else {
        struct RemoteFilter* curr = *remoteFilters;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newRemoteFilter;
    }   
}

void removeDataForDeletedFiltersFromThisHost(char* host, struct RemoteFilter* remoteFilter){
    struct Filter* filtersForHost = readFiltersForHost(host);
    while(remoteFilter != NULL){
     /* Take a copy of all the filters that currently exist for this host in the database
        and blank out the name of each one that came through in the data from the current
        sync. We will use these in a moment when deciding which filters can be deleted 
        because they no longer exist on the remote host. */
        struct Filter* filter = getFilterFromId(filtersForHost, remoteFilter->localId);     
        if (filter != NULL){
            free(filter->name);
            filter->name = NULL;
        }
        remoteFilter = remoteFilter->next;  
    }
    struct Filter* filter = filtersForHost;
    while (filter != NULL) {
        if (filter->name != NULL){
            int result = removeFilter(filter->name, host);
            if (result == FAIL){
             // continue with the others if one fails - we can try again on the next sync
                statusMsg("Failed to delete obsolete remote filter '%s' for host %s\n", filter->name, host);
            }
        }       
        filter = filter->next;  
    }
    
    if (filtersForHost != NULL){
        freeFilters(filtersForHost);
    }
}

int readLine(SOCKET fd, char* line){
    int prevChar = 0, thisChar = 0;
    int lineIndex = 0;
    int rc;
    while ((rc = RECV(fd, (line + lineIndex), 1, 0)) > 0) {
        thisChar = line[lineIndex++];
        if (thisChar== '\n' && prevChar == '\r'){
         // We found an HTTP end-of-line sequence, so the current line has been read. Insert a string terminator and return.
            line[lineIndex] = 0;
            return TRUE;

        } else {
         // Not at the end of a line yet, keep reading
            prevChar = thisChar;
        }

        if (lineIndex > MAX_LINE_LEN){
         /* This is unlikely to have come from the BitMeter server, possibly a long header added
            in by a proxy en-route, we can just truncate it and continue */
            line[MAX_LINE_LEN] = 0;
            return TRUE;
        }
    }
    if (rc < 0 && errno > 0){
     // There was a problem reading from the socket
        statusMsg("ERR %d %d %s\r\n", rc, errno, strerror(errno));
    }

    return FALSE;
}

int httpHeadersOk(SOCKET fd){
    char line[MAX_LINE_LEN + 1];

 // First read all the HTTP headers
    while(readLine(fd, line)) {
        if (startsWith(line, "HTTP")) {
         // Check the server returned an HTTP 200 response code
            char responseCode[4];
            sscanf(line, "%*s %s %*s", responseCode);
            if (strcmp("200", responseCode) != 0){
                statusMsg("Bad HTTP response code: %s", responseCode);
                return FAIL;
            }

        } else if (startsWith(line, HEADER_CONTENT_TYPE)) {
         // Check that the content-type is what we expect
            if (strstr(line, SYNC_CONTENT_TYPE) == NULL){
                statusMsg("Bad content type: %s", line);
                return FAIL;
            }

        } else if (startsWith(line, HTTP_EOL)) {
         // Reached the end of the headers
            break;
        }
    }   

    return SUCCESS;
}

int parseData(SOCKET fd, char* alias, time_t prevMaxTsForHost, int* rowCount){
 // Handle the response that is returned from the host
    int result = httpHeadersOk(fd);
    if (result == FAIL){
     // There was a problem with the headers, so stop now
        return FAIL;
    }

    struct RemoteFilter* remoteFilters = NULL;
    char line[MAX_LINE_LEN + 1];
    
 // We want one transaction per host, insertion of all the data for a single host must be atomic
	beginTrans(TRUE);
	
 /* There is a small chance that something added rows for the host we are currently syncing with
	since we last checked the database (which was before the sync request was sent). Check the db again
	within the same transaction that we will use to insert the new values, and bail out if things have
	changed. */
    time_t currMaxTsForHost = getMaxTsForHost(alias);
    
	if (currMaxTsForHost != prevMaxTsForHost) {
        statusMsg("Data for host %s has changed in local database during sync - aborting", alias);
        result = FAIL;
		
    } else {
		//TODO check correct version of bm
	 // Next read the filters
		while(readLine(fd, line)) {
			if (startsWith(line, FILTER_ROW_PREFIX)){
				struct Filter* filterFromRow = parseFilterRow(line, alias);
				int localFilterId = getLocalFilter(filterFromRow);
				
				struct RemoteFilter* remoteFilter = allocRemoteFilter(localFilterId, filterFromRow->id);
				appendRemoteFilter(&remoteFilters, remoteFilter);
				freeFilters(filterFromRow);
				
			} else {
				break;  
			}
		}
	}
		
    removeDataForDeletedFiltersFromThisHost(alias, remoteFilters);

 // Read the data one row at a time
    struct Data* data;
    int resultCount = 0, rc;

    do {
        data = parseDataRow(line);
        if (data == NULL){
            statusMsg("Malformed data returned from host");
            result = FAIL;
            break;
        }

        int localFilterId = getLocalId(remoteFilters, data->fl);
        if (localFilterId == 0){
            statusMsg("Unknown filter id in host data"); //TODO display id
            result = FAIL;
            break;
        } else {
            data->fl = localFilterId;
        }
        
        rc = insertData(data);
        if (rc == FAIL){
            statusMsg("Unable to insert data into local db");
            result = FAIL;
            break;
        }
        freeData(data);
        data = NULL;
        resultCount++;
    } while(readLine(fd, line));
    
	if (result == SUCCESS){
	    commitTrans();   
	} else {
	    rollbackTrans();
	}
	
    if (data != NULL){
        freeData(data); 
    }
    if (remoteFilters != NULL){
        freeRemoteFilters(remoteFilters);
    }

    *rowCount = resultCount;

    return result;
}

int sendRequest(SOCKET fd, time_t ts, char* host, int port){
 // Send an HTTP request to the specified host/port asking for any data newer than 'ts'
    char buffer[MAX_REQUEST_LEN];
    sprintf(buffer, "GET /sync?ts=%d HTTP/1.1" HTTP_EOL, (int)ts);

    SEND(fd, buffer, strlen(buffer), 0);

    if (port == DEFAULT_HTTP_PORT){
        sprintf(buffer, "Host: %s", host);
    } else {
        sprintf(buffer, "Host: %s:%d", host, port);
    }
    SEND(fd, buffer, strlen(buffer), 0);
    
    sprintf(buffer, HTTP_EOL HTTP_EOL);
    SEND(fd, buffer, strlen(buffer), 0);

    return SUCCESS;
}
