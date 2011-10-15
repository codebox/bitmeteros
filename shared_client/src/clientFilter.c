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

#define SQL_SELECT_FILTERS          "SELECT id, desc, name, expr, host FROM filter"  
#define SQL_INSERT_FILTER              "INSERT INTO filter (id, desc, name, expr, host) VALUES (?,?,?,?,?)"
#define SQL_GET_NEXT_FILTER_ID         "SELECT MAX(id) AS id FROM filter"
#define SQL_SELECT_FILTER_BY_NAME_HOST "SELECT id FROM filter WHERE upper(name)=upper(?) and host=?"
#define SQL_SELECT_FILTER_BY_NAME  	"SELECT id FROM filter WHERE upper(name)=upper(?) and host is NULL"
#define SQL_DELETE_FILTER_BY_ID    	"DELETE FROM filter WHERE id=?"
#define SQL_DELETE_DATA_FOR_FILTER 	"DELETE FROM data2 WHERE fl=?"

#define DUMMY_VALID_IP "1.2.3.4"

static struct Filter* makeFilterFromRow(sqlite3_stmt* stmt){
	int   id   = sqlite3_column_int(stmt,   0);
	char* desc = sqlite3_column_text(stmt,  1);
	char* name = sqlite3_column_text(stmt,  2);
	char* expr = sqlite3_column_text(stmt,  3);
	char* host = sqlite3_column_text(stmt,  4);

	return allocFilter(id, desc, name, expr, host);    
}

struct Filter* readFiltersForHost(char* host) {
	struct Filter* filtersForHost = NULL;
	struct Filter* allFilters = readFilters();
	struct Filter* thisFilter = allFilters;
	
	while (thisFilter != NULL) {
		if (filterHasHost(thisFilter, host)) {
			appendFilter(&filtersForHost, copyFilter(thisFilter));
		}
		thisFilter = thisFilter->next;	
	}
	
	freeFilters(allFilters);
	
	return filtersForHost;
}

struct Filter* readFilters(){
	int rc;
	
	struct Filter* filters = NULL;
	sqlite3_stmt* stmtReadFilters = getStmt(SQL_SELECT_FILTERS);
	
	while ((rc=sqlite3_step(stmtReadFilters)) == SQLITE_ROW) {
		struct Filter* filter = makeFilterFromRow(stmtReadFilters);
		appendFilter(&filters, filter);
	}
  	finishedStmt(stmtReadFilters);
  	
	return filters;
}

int filterExprIsValid(char* expr1){
	struct bpf_program fcode;
		 	
 // Substitute dummy values for our custom filter expression values, 
 	char* expr2 = replace(expr1, LAN_IPS,     DUMMY_VALID_IP);
 	char* expr3 = replace(expr2, ADAPTER_IPS, DUMMY_VALID_IP);

	int result; 	
	if (pcap_compile_nopcap(100, DLT_EN10MB, &fcode, expr3, 1, 0) < 0){
		result = FAIL;
	} else {
		result = SUCCESS;
	}	
	free(expr2);
	free(expr3);
	return result;
}

struct Filter* getFilter(char* name, char* host){
	struct Filter* allFilters = readFilters();
	
	struct Filter* match = getFilterFromName(allFilters, name, host);
	struct Filter* result = NULL;
	if (match != NULL){
		result = copyFilter(match);
	}	
	freeFilters(allFilters);
	
	return result;
}

int addFilter(struct Filter* filter){
	beginTrans(TRUE);
	int nextFilterId = getNextId(SQL_GET_NEXT_FILTER_ID);

	sqlite3_stmt *insertStmt = getStmt(SQL_INSERT_FILTER);
	sqlite3_bind_int(insertStmt,  1, nextFilterId);
	sqlite3_bind_text(insertStmt, 2, filter->desc, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(insertStmt, 3, filter->name, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(insertStmt, 4, filter->expr, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(insertStmt, 5, filter->host, -1, SQLITE_TRANSIENT);

	int rc = sqlite3_step(insertStmt);
	finishedStmt(insertStmt);
	
	if (rc == SQLITE_DONE) {
		commitTrans();
		return nextFilterId;
	} else {
		rollbackTrans();
		return FAIL;
	}
}

int removeFilter(char* name, char* host){
	beginTrans(TRUE);
	
	sqlite3_stmt *selectStmt;
	if (host==NULL){
		selectStmt = getStmt(SQL_SELECT_FILTER_BY_NAME);	
		sqlite3_bind_text(selectStmt, 1, name, -1, SQLITE_TRANSIENT);
		
	} else {
		selectStmt = getStmt(SQL_SELECT_FILTER_BY_NAME_HOST);	
		sqlite3_bind_text(selectStmt, 1, name, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(selectStmt, 2, host, -1, SQLITE_TRANSIENT);
	}
        
    int rc = sqlite3_step(selectStmt);
    int status = (rc == SQLITE_ROW);
    int filterId;
	sqlite3_stmt *deleteStmt;
    
	if (status == SUCCESS){
		filterId = sqlite3_column_int(selectStmt, 0);

	 // Delete bandwidth data for the filter
		deleteStmt = getStmt(SQL_DELETE_DATA_FOR_FILTER);
		sqlite3_bind_int(deleteStmt, 1, filterId);

		rc = sqlite3_step(deleteStmt);
		if (rc == SQLITE_DONE){
			status = SUCCESS;		
		} else {
			status = FAIL;	
		}
		finishedStmt(deleteStmt);
	}
	finishedStmt(selectStmt);
	
	if (status == SUCCESS) {			 
	 // Delete the filter itself
		deleteStmt = getStmt(SQL_DELETE_FILTER_BY_ID);
		sqlite3_bind_int(deleteStmt, 1, filterId);
				
		rc = sqlite3_step(deleteStmt);
		if (rc == SQLITE_DONE){
			status = SUCCESS;		
		} else {
			status = FAIL;	
		}
		finishedStmt(deleteStmt);
	}

	if (status == SUCCESS){
		commitTrans();
	} else {
		rollbackTrans();	
	}

    return status;		
}