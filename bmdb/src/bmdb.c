/*
 * BitMeterOS v0.2.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Build Date: Wed, 25 Nov 2009 10:48:23 +0000
 */

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
static int doWebLocal(FILE* file, int argc, char** argv);

// This struct represents an action that can be performed by this utility
struct Action{
    char* name;
    char* description;
    int (*fn)(FILE*, int, char**);
};

// The 'name' values are specified on the command-line by the user
struct Action actions[] = {
    {"showconfig", "Displays all configuration values", &doConfig},
    {"vac",        "Vacuums the database, freeing unused space", &doVacuum},
    {"version",    "Displays version information", &doVersion},
    {"upgrade",    "Upgrades the database", &doUpgrade},
    {"webremote",  "Enable remote access to the web interface", &doWebRemote},
    {"weblocal",   "Disable remote access to the web interface", &doWebLocal},
    {NULL, NULL, NULL}
};

int main(int argc, char **argv){
    printf(COPYRIGHT);

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
            prepareDb();
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

static int doVacuum(){
 // Frees any unused space occupied by the database file
    printf("Vacuuming database...\n");
	int status = executeSql("VACUUM", NULL);
	printf("Finished.\n");

	return status;
}

static int doWebRemote(FILE* file, int argc, char** argv){
 // Allow remote access to the web interface
    int webRemoteValue = getConfigInt(CONFIG_WEB_ALLOW_REMOTE);
    int status;
    if (webRemoteValue == TRUE){
        printf("Remote access to the web interface is already enabled\n");
        status = FAIL;
    } else {
        setConfigIntValue(CONFIG_WEB_ALLOW_REMOTE, TRUE);
        printf("Remote access to the web interface will be enabled next time the bmws utility is started.");
        status = SUCCESS;
    }
    return status;
}

static int doWebLocal(FILE* file, int argc, char** argv){
 // Disallow remote access to the web interface
    int webRemoteValue = getConfigInt(CONFIG_WEB_ALLOW_REMOTE);
    int status;
    if (webRemoteValue == FALSE){
        printf("Remote access to the web interface is already disabled\n");
        status = FAIL;
    } else {
        setConfigIntValue(CONFIG_WEB_ALLOW_REMOTE, FALSE);
        printf("Remote access to the web interface will be disabled next time the bmws utility is started.");
        status = SUCCESS;
    }
    return status;
}

static int dumpActions(){
 // Display a list of the available actions
    printf("The following actions are available:" EOL EOL);
	struct Action* action = actions;
	while(action->name != NULL){
        printf(" %-12s - %s\n", action->name, action->description);
        action++;
	}

	return SUCCESS;
}
