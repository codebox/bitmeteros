#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "bmclient.h"

/*
Contains the entry-point for the bmclient command-line utility.
*/

// This struct get populated by the various flags/options read from the command-line
struct Prefs prefs = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL};

#ifndef UNIT_TESTING    
    int main(int argc, char **argv){
        return _main(argc, argv);   
    }
#endif

int _main(int argc, char **argv){
 // Interpret the command-lin arguments and decide if they make sense
    int status = PARSE_ARGS(argc, argv, &prefs);

    SET_LOG_LEVEL(LOG_INFO);    
    OPEN_DB();
    DB_VERSION_CHECK();

    if (status == FAIL){
        showCopyright();
        
     // The command-line was duff...
        if (prefs.errorMsg != NULL){
         // ...and we have a specific error to show the user
            PRINT(COLOUR_RED, "Error: %s\n", prefs.errorMsg);
        } else {
         // ...and we have no specific error message, so show a vague one
            PRINT(COLOUR_RED, ERR_WTF EOL);
        }
        printf(SHOW_HELP);

    } else if (prefs.help){
        showCopyright();
        
     // Dump the help info and stop
        DO_HELP();

    } else if (prefs.version){
        showCopyright();
        
     // Show the version and stop
        DO_VERSION();

    } else {
        switch(prefs.mode){
            case PREF_MODE_DUMP:
                DO_DUMP();
                break;
            case PREF_MODE_SUMMARY:
                showCopyright();
                DO_SUMMARY();
                break;
            case PREF_MODE_MONITOR:
                showCopyright();
                DO_MONITOR();
                break;
            case PREF_MODE_QUERY:
                showCopyright();
                DO_QUERY();
                break;
            default:
                assert(FALSE); // Any other mode value should cause parseArgs to fail
                break;
        }

    }
    CLOSE_DB();
    
    return 0;
}
