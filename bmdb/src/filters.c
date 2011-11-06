#include <stdio.h>
#include "common.h"
#include "bmdb.h"

int showFilters(FILE* file, int argc, char** argv){
    struct Filter* filter = readFilters();
    
    int gotFilters = FALSE;
    int maxFilterDescWidth  = getMaxFilterDescWidth(filter);
    if (maxFilterDescWidth < 11){
        maxFilterDescWidth = 11;    
    }
    int maxFilterNameWidth  = getMaxFilterNameWidth(filter);
    if (maxFilterNameWidth < 4){
        maxFilterNameWidth = 4; 
    }
    
    PRINT(BMDB_COL_1, "%-*s  %-*s  Filter\n", maxFilterDescWidth, "Description", maxFilterNameWidth, "Name");
    PRINT(BMDB_COL_1, "%-*s  %-*s  ------\n", maxFilterDescWidth, "-----------", maxFilterNameWidth, "----");
    
    while (filter != NULL){
        gotFilters = TRUE;
        PRINT(BMDB_COL_1, "%-*s  ", maxFilterDescWidth, filter->desc);
        PRINT(BMDB_COL_2, "%-*s  %s\n", maxFilterNameWidth, filter->name, filter->expr);
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
            PRINT(COLOUR_RED, "Error - no filter named '%s' has been defined.", filterName);
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
                    PRINT(COLOUR_RED, "Error - failed to delete the filter '%s'", filterName);
                }
        
            } else {
                printf("Action aborted, no data deleted.", filterName);
                status = FAIL;
            }
        }
                
    } else if (argc > 1) {
        PRINT(COLOUR_RED, "Error - you must enter the NAME of just one filter that you wish to delete. ");
        PRINT(COLOUR_RED, "If the name contains spaces, it must be enclosed in quotes.");
        status = FAIL;
        
    } else {
        PRINT(COLOUR_RED, "Error - you must enter the NAME of the filter that you wish to delete.");
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

     // Check that the filter name is valid
        if (!filterNameIsValid(filterName)){
            PRINT(COLOUR_RED, "Error - the specified filter name '%s' is not valid - please pick a name using only letters and numbers, between 1 and %d characters in length.", filterName, FILTER_NAME_MAX_LENGTH);  
            status = FAIL;
        } else {
            status = SUCCESS;
        }

     // Check that the name is not already in use
        if (status == SUCCESS){
            if (getFilter(filterName, NULL) != NULL){
                PRINT(COLOUR_RED, "Error - a filter named '%s' already exists, please pick another name.", filterName);
                status = FAIL;
            }
        }
        
        if (status == SUCCESS){
         // Check that the filter expression is valid
            if (!filterExprIsValid(filterTxt)){
                PRINT(COLOUR_RED, "Error - unable to compile the filter expression '%s' - please check your syntax.", filterTxt);  
                status = FAIL;
            } else {
                status = SUCCESS;
            }
        }

        if (status == SUCCESS){
            struct Filter filter = {0, filterDesc, filterName, filterTxt, NULL, NULL};
            int filterId = addFilter(&filter);
            
            if (filterId == FAIL){
                PRINT(COLOUR_RED, "Error - failed to add new filter into the database.");
                status = FAIL;
            } else {
                printf("New filter added ok. The filter will be activated next time the Capture process is started.");
                status = SUCCESS;
            }
        }

    } else {
        PRINT(COLOUR_RED, "Error - expected exactly 3 arguments, the name of the new filter, a textual description, and the filter expression itself (see bmdb help for examples).");
        status = FAIL;
    }
    
    return status;
}