#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "common.h"
#include "sqlite3.h"
#include "bmdb.h"

/*
Contains the entry-point for the bmdb utility, which performs admin/config operations
on the BitMeterOS database.
*/

static struct Action* getActionForName(char* name);
static int doVacuum();
static int dumpActions();
static int doWebRemote(FILE* file, int argc, char** argv);
static int doWebRemoteAdmin(FILE* file, int argc, char** argv);
static int doWebLocal(FILE* file, int argc, char** argv);
static int doHelp();
static int doPurge();

// This struct represents an action that can be performed by this utility
struct Action{
    char* name;
    char* description;
    int (*fn)(FILE*, int, char**);
};

// The 'name' values are specified on the command-line by the user
struct Action actions[] = {
    {"showconfig",     "Displays all configuration values", &doListConfig},
    {"setconfig",      "Adds or updates a configuration value", &doSetConfig},
    {"rmconfig",       "Removes a configuration value", &doRmConfig},
    {"vac",            "Vacuums the database, freeing unused space", &doVacuum},
    {"version",        "Displays version information", &doVersion},
    {"upgrade",        "Upgrades the database", &doUpgrade},
    {"weblocal",       "Disable all remote access to the web interface", &doWebLocal},
    {"webremote",      "Enable non-administrative remote access to the web interface", &doWebRemote},
    {"webremoteadmin", "Enable administrative remote access to the web interface", &doWebRemoteAdmin},
    {"webstop",        "Stop the web server process", &doWebStop},
    {"webstart",       "Start the web server process", &doWebStart},
    {"capstop",        "Stop the data capture process", &doCapStop},
    {"capstart",       "Start the data capture process", &doCapStart},
    {"purge",          "Delete all bandwidth data from the database", &doPurge},
    {"showfilters",    "Lists all packet filters", &showFilters},
    {"addfilter",      "Adds a new packet filter", &addFilter},
    {"rmfilter",       "Deletes the named packet filter, and all its data", &rmFilter},
    {"help",           "Displays full help text", &doHelp},
    {NULL, NULL, NULL}
};

int main(int argc, char **argv){
    printf(COPYRIGHT);
    setLogLevel(LOG_INFO);

	if (argc == 1){
     // If the utility is called without an action argument then display the list of available actions
        dumpActions();
        return SUCCESS;

	} else {
     // Find the Action struct that matches the command-line argument
        char* name = argv[1];
        struct Action* action = getActionForName(name);
        if (action == NULL){
         // Never heard of it
            printf(ERR_BAD_ACTION "\n");
            return FAIL;

        } else {
         // We found a match
            openDb();
            int status = (*action->fn)(stdout, argc-2, argv+2);
            closeDb();
            return status;
        }
	}
}

static struct Action* getActionForName(char* name){
 // Find the Action that has the specified name
    struct Action* namedAction = NULL;
    struct Action* action = actions;

	while(action->name != NULL){
        if (strcmp(name, action->name) == 0){
         // Found it
            namedAction = action;
            break;
        }
        action++;
	}

	return namedAction;
}

static int doPurge(){
 // Deletes all bandwidth data from the database
    printf("This action will delete ALL BitMeter data, and cannot be undone.\nAre you sure you want to proceed? (Enter Y/N): ");
    
    int status = SUCCESS;
    int c = getchar();
    if (c == 'y' || c == 'Y'){
    	status = executeSql("DELETE FROM data", NULL);
    	if (status == SUCCESS){
    		printf("Data deleted.");
    	} else {
    		printf("Unable to delete data.");
    	}
    } else {
    	printf("Action aborted, no data deleted.");
    }

	return status;
}

static int doVacuum(){
 // Frees any unused space occupied by the database file
    printf("Vacuuming database...\n");
	int status = executeSql("VACUUM", NULL);
	printf("Finished.\n");

	return status;
}

static int setAccessLevel(int level, char* successMsg, char* failMsg){
    int currentAccessLevel = getConfigInt(CONFIG_WEB_ALLOW_REMOTE, 0);
    int status;
    if (currentAccessLevel == level){
        printf(failMsg);
        status = FAIL;
    } else {
        setConfigIntValue(CONFIG_WEB_ALLOW_REMOTE, level);
        printf(successMsg);
        status = SUCCESS;
    }
    return status;
}

static int doWebLocal(FILE* file, int argc, char** argv){
 // Disallow all remote access to the web interface
    return setAccessLevel(ALLOW_LOCAL_CONNECT_ONLY,
	    "Remote access to the web interface will be disabled next time the bmws utility is started.",
	    "Remote access to the web interface is already disabled\n");
}

static int doWebRemote(FILE* file, int argc, char** argv){
 // Allow non-administrative remote access to the web interface
    return setAccessLevel(ALLOW_REMOTE_CONNECT, 
	    "Non-administrative remote access to the web interface will be enabled next time the bmws utility is started.",
	    "Non-administrative remote access to the web interface is already enabled\n");
}

static int doWebRemoteAdmin(FILE* file, int argc, char** argv){
 // Allow administrative remote access to the web interface
    return setAccessLevel(ALLOW_REMOTE_ADMIN,
    	"Administrative remote access to the web interface will be enabled next time the bmws utility is started.",
    	"Administrative remote access to the web interface is already enabled\n");
}

extern char* helpTxt;
static int doHelp(){
	printf(helpTxt);
	return SUCCESS;
}

static int dumpActions(){
 // Display a list of the available actions
    printf("The following actions are available:" EOL EOL);
	struct Action* action = actions;
	while(action->name != NULL){
        printf(" %-14s - %s\n", action->name, action->description);
        action++;
	}

	return SUCCESS;
}
