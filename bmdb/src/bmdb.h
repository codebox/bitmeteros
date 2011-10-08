#include <stdio.h>

#define ERR_OPT_NO_ARGS       "No arguments were supplied"
#define ERR_BAD_ACTION        "Unrecognised action"
#define ERR_BAD_UPGRADE_LEVEL "Invalid upgrade level specified"
#define INFO_DUMPING_CONFIG   "Dumping config table..."

#define MAX_UPGRADE_LEVEL 8

int doVersion();
int convertAddrValues();
int doListConfig(FILE* file, int argc, char** argv);
int doSetConfig(FILE* file, int argc, char** argv);
int doRmConfig(FILE* file, int argc, char** argv);
int doUpgrade(FILE* file, int argc, char** argv);
int setConfigIntValue(char* key, int value);
int doWebStop(FILE* file, int argc, char** argv);
int doWebStart(FILE* file, int argc, char** argv);
int doCapStop(FILE* file, int argc, char** argv);
int doCapStart(FILE* file, int argc, char** argv);
int showFilters(FILE* file, int argc, char** argv);
int rmFilter(FILE* file, int argc, char** argv);
int addFilter(FILE* file, int argc, char** argv);
