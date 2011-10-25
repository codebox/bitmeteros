#include <stdio.h>
#include "common.h"

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
		
		if (getFilter(filterName, NULL) != NULL){
			status = SUCCESS;
		} else {
			printf("Error - no filter named '%s' has been defined.", filterName);
			status = FAIL;
		}

		if (status == SUCCESS){		
			printf("This action will permanently delete ALL data captured using this filter.\n");
			printf("Are you sure you want to proceed? (Enter Y/N): ");
			
			int c = getchar();
    		if (c == 'y' || c == 'Y'){
				status = removeFilter(filterName, NULL);
				
				if (status == SUCCESS){
					printf("The filter '%s' has been deleted.", filterName);
				} else {
					printf("Error - failed to delete the filter '%s'", filterName);
				}
    	
	    	} else {
	    		printf("Action aborted, no data deleted.", filterName);
				status = FAIL;
	    	}
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

int addNewFilter(FILE* file, int argc, char** argv){
	int status;
	
	if (argc == 3){
		char* filterName = argv[0];
		char* filterDesc = argv[1];
		char* filterTxt  = argv[2];

	 // Check that the name is not already in use
	 	if (getFilter(filterName, NULL) != NULL){
			printf("Error - a filter named '%s' already exists, please pick another name.", filterName);
			status = FAIL;
	 	} else {
			status = SUCCESS;	 		
	 	}
		
		if (status == SUCCESS){
		 // Check that the filter expression is valid
			if (filterExprIsValid(filterTxt)){
				printf("Error - unable to compile the filter expression '%s' - please check your syntax.", filterTxt);	
				status = FAIL;
			} else {
				status = SUCCESS;
			}
		}
		
		if (status == SUCCESS){
			struct Filter filter = {0, filterDesc, filterName, filterTxt, NULL, NULL};
			int filterId = addFilter(&filter);
			
			if (filterId == FAIL){
				printf("Error - failed to add new filter into the database.");
				status = FAIL;
			} else {
				printf("New filter added ok. The filter will be activated next time the Capture process is started.");
				status = SUCCESS;
			}
		}

	} else {
		printf("Error - expected exactly 3 arguments, the name of the new filter, a textual description, and the filter expression itself (see bmdb help for examples).");
		status = FAIL;
	}
	
	return status;
}