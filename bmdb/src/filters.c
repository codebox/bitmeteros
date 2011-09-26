#include <stdio.h>
#include "common.h"

#define DUMMY_VALID_IP "1.2.3.4"

#define SQL_SELECT_FILTER_BY_NAME  	"SELECT id FROM filter WHERE upper(name)=upper(?)"
#define SQL_DELETE_FILTER_BY_ID    	"DELETE FROM filter WHERE id=?"
#define SQL_DELETE_DATA_FOR_FILTER 	"DELETE FROM data2 WHERE fl=?"
#define SQL_INSERT_FILTER 			"INSERT INTO filter (desc, name, expr) VALUES (?,?,?)"

int showFilters(FILE* file, int argc, char** argv){
	struct Filter* filter = readFilters();
	
	int gotFilters = FALSE;
	int maxFilterDescWidth	= getMaxFilterDescWidth(filter);
	if (maxFilterDescWidth < 11){
		maxFilterDescWidth = 11;	
	}
	int maxFilterNameWidth	= getMaxFilterNameWidth(filter);
	if (maxFilterNameWidth < 4){
		maxFilterNameWidth = 4;	
	}
	
	printf("%-*s  %-*s  Filter\n", maxFilterDescWidth, "Description", maxFilterNameWidth, "Name");
	printf("%-*s  %-*s  ------\n", maxFilterDescWidth, "-----------", maxFilterNameWidth, "----");
	
	while (filter != NULL){
		gotFilters = TRUE;
		printf("%-*s  %-*s  %s\n", maxFilterDescWidth, filter->desc, maxFilterNameWidth, filter->name, filter->expr);
		filter = filter->next;	
	}
	
	if (!gotFilters) {
		printf("No filters have been defined\n");	
	}
	
	return SUCCESS;
}

int rmFilter(FILE* file, int argc, char** argv){
	int status;
	
	if (argc == 1){
		char* filterName = argv[0];
		
		beginTrans(TRUE);
		
		sqlite3_stmt *selectStmt = getStmt(SQL_SELECT_FILTER_BY_NAME);
		sqlite3_bind_text(selectStmt, 1, filterName, -1, SQLITE_TRANSIENT);
        
        int rc = sqlite3_step(selectStmt);
		if (rc == SQLITE_ROW){
			printf("This action will permanently delete ALL data captured using this filter.\n");
			printf("Are you sure you want to proceed? (Enter Y/N): ");
			
			int c = getchar();
    		if (c == 'y' || c == 'Y'){
				int filterId = sqlite3_column_int(selectStmt, 0);

				sqlite3_stmt *deleteStmt;
				
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
					printf("The filter '%s' has been deleted.", filterName);
				} else {
					printf("Error - failed to delete the filter '%s'. rc=%d", filterName, rc);
				}
    	
	    	} else {
	    		printf("Action aborted, no data deleted.", filterName);
				status = FAIL;
	    	}
			
		} else {
			printf("Error - no filter named '%s' has been defined.", filterName);
			status = FAIL;
		}

	  	finishedStmt(selectStmt);
	  	
		if (status == SUCCESS){
			commitTrans();
		} else {
			rollbackTrans();
		}
		
	} else if (argc > 1) {
		printf("Error - you must enter the NAME of just one filter that you wish to delete. ");
		printf("If the name contains spaces, it must be enclosed in quotes.");
		status = FAIL;
		
	} else {
		printf("Error - you must enter the NAME of the filter that you wish to delete.");
		status = FAIL;
	}
	
	return status;
}

int addFilter(FILE* file, int argc, char** argv){
	int status;
	
	if (argc == 3){
		char* filterName = argv[0];
		char* filterDesc = argv[1];
		char* filterTxt  = argv[2];
		
	 // Check that the name is not already in use
		sqlite3_stmt *selectStmt = getStmt(SQL_SELECT_FILTER_BY_NAME);
		sqlite3_bind_text(selectStmt, 1, filterName, -1, SQLITE_TRANSIENT);
		int rc = sqlite3_step(selectStmt);
		if (rc == SQLITE_ROW){
			printf("Error - a filter named '%s' already exists, please pick another name.", filterName);
			status = FAIL;
		} else {
			status = SUCCESS;
		}
		
		if (status == SUCCESS){
		 // Check that the filter expression is valid
		 	struct bpf_program *fcode;
		 	
		 // Substitute dummy values for our custom filter expression values, 
		 	char* tmpFilterTxt = filterTxt;
		 	tmpFilterTxt = replace(tmpFilterTxt, LAN_IPS,     DUMMY_VALID_IP);
		 	tmpFilterTxt = replace(tmpFilterTxt, ADAPTER_IPS, DUMMY_VALID_IP);
		 	
			if (pcap_compile_nopcap(100, DLT_EN10MB, &fcode, tmpFilterTxt, 1, 0) < 0){
				printf("Error - unable to compile the filter expression '%s' - please check your syntax.", filterTxt);	
				status = FAIL;
			} else {
				status = SUCCESS;
			}
		}
		
		if (status == SUCCESS){
			sqlite3_stmt *insertStmt = getStmt(SQL_INSERT_FILTER);
			sqlite3_bind_text(insertStmt, 1, filterDesc, -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(insertStmt, 2, filterName, -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(insertStmt, 3, filterTxt,  -1, SQLITE_TRANSIENT);
			
			int rc = sqlite3_step(insertStmt);
			if (rc == SQLITE_DONE){
				printf("New filter added ok. The filter will be activated next time the Capture process is started.");
				status = SUCCESS;
			} else {
				printf("Error - failed to add new filter into the database. rc=%d", rc);
				status = FAIL;
			}
		}

	} else {
		printf("Error - expected exactly 3 arguments, the name of the new filter, a textual description, and the filter expression itself (see bmdb help for examples).");
		status = FAIL;
	}
	
	return status;
}