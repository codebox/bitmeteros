#define _GNU_SOURCE
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "bmsync.h"

static int setPort(struct SyncPrefs* prefs, char* portTxt);
static int setAlias(struct SyncPrefs* prefs, char* alias);

int parseSyncArgs(int argc, char **argv, struct SyncPrefs *prefs){
	char OPT_LIST[11];
	sprintf(OPT_LIST, "%c%c%c:%c:", OPT_HELP, OPT_VERSION, OPT_PORT, OPT_ALIAS);

	int status = SUCCESS;

	if (argc <= 1){
	 // The command line was empty
		prefs->errMsg = strdup(ERR_OPT_NO_ARGS);
		status = FAIL;

	} else {
		int opt;
		while ((opt = getopt(argc, argv, OPT_LIST)) != -1){
			switch (opt){
				case OPT_HELP:
					prefs->help = 1;
					status = SUCCESS;
					break;

				case OPT_VERSION:
					prefs->version = 1;
					status = SUCCESS;
					break;

				case OPT_PORT:
					status = setPort(prefs, optarg);
					break;

				case OPT_ALIAS:
					status = setAlias(prefs, optarg);
					break;

				default:
					status = FAIL;
					break;
			}
		 // Check the 'status' value after each option, and stop if we see anything invalid
			if (status == FAIL){
				break;
			}
		}

		if (status == SUCCESS && prefs->version == 0 && prefs->help == 0){
			if (optind == argc){
				prefs->errMsg = strdup(ERR_NO_HOST);
				status = FAIL;

			} else {
				if ((optind < argc - 1) && prefs->alias != NULL){
					// multiple hosts with same alias, probably an error so stop
					prefs->errMsg = strdup(ERR_MULTIPLE_HOSTS_ONE_ALIAS);
					status = FAIL;

				} else {
				    int hostCount = argc - optind;
				    char **hosts = malloc(sizeof(char *) * hostCount);
				    int i;
				    for(i=optind; i<argc; i++){
                        hosts[i - optind] = strdup(argv[i]);
				    }
                    prefs->hosts = hosts;
					prefs->hostCount = hostCount;
				}
			}
		}
	}
	return status;
}

static int setPort(struct SyncPrefs* prefs, char* portTxt){
	int port = atoi(portTxt);
	if (port < 1 || port > 65535){
		prefs->errMsg = strdup(ERR_BAD_PORT);
		return FAIL;
	} else {
		prefs->port = port;
		return SUCCESS;
	}
}

static int setAlias(struct SyncPrefs* prefs, char* alias){
	prefs->alias = strdup(alias);
	return SUCCESS;
}
