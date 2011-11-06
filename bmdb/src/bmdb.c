#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "common.h"
#include "sqlite3.h"
#include "bmdb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
    {"addfilter",      "Adds a new packet filter", &addNewFilter},
    {"rmfilter",       "Deletes the named packet filter, and all its data", &rmFilter},
    {"help",           "Displays full help text", &doHelp},
    {NULL, NULL, NULL}
};

int main(int argc, char **argv){
    openDb();
    showCopyright();
    setLogLevel(LOG_INFO);
    
    int status;
    
    if (argc == 1){
     // If the utility is called without an action argument then display the list of available actions
        dumpActions();
        status = SUCCESS;

    } else {
     // Find the Action struct that matches the command-line argument
        char* name = argv[1];
        struct Action* action = getActionForName(name);
        if (action == NULL){
         // Never heard of it
            PRINT(COLOUR_RED, ERR_BAD_ACTION "\n");
            status = FAIL;

        } else {
         // We found a match
            status = (*action->fn)(stdout, argc-2, argv+2);
        }
    }
    closeDb();
    return status;
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
            PRINT(COLOUR_RED, "Unable to delete data.");
        }
    } else {
        printf("Action aborted, no data deleted.");
    }

    return status;
}

static int doVacuum(){
 // Frees any unused space occupied by the database file
    int dbFileSize1 = getDbFileSize();
    printf("Database is currently %dk in size, vacuuming...", dbFileSize1/1024);
    int status = executeSql("VACUUM", NULL);
    printf("finished.\n");
    
    int dbFileSize2 = getDbFileSize();
    int diff = dbFileSize1 - dbFileSize2;

    if (diff == 0){
        printf("The database file could not be compressed any further - no space was saved.\n");    
    } else {
        printf("Compressed database to %dk, saving ", dbFileSize2/1024);
        if (diff < 1024) {
            printf("%d bytes\n", diff);         
        } else {
            printf("%dk\n", diff/1024);
        }
    }

    return status;
}

static int getDbFileSize(){
    char dbPath[MAX_PATH_LEN];
    getDbPath(dbPath);
    
    FILE* dbFile = fopen(dbPath, "r");
    int fd = fileno(dbFile);
    struct stat buf;
    fstat(fd, &buf);
    int size = buf.st_size;
    
    fclose(dbFile);
    
    return size;
}
static int setAccessLevel(int level, char* successMsg, char* failMsg){
    int currentAccessLevel = getConfigInt(CONFIG_WEB_ALLOW_REMOTE, 0);
    int status;
    if (currentAccessLevel == level){
        PRINT(COLOUR_RED, failMsg);
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
        PRINT(BMDB_COL_1, " %-14s", action->name);
        PRINT(BMDB_COL_2, " - %s\n", action->description);
        action++;
    }

    return SUCCESS;
}
