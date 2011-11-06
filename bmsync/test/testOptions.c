#define _GNU_SOURCE
#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include "string.h"
#include "getopt.h"
#include "bmsync.h"

/*
Contains unit tests for the options module of the bmsync utility.
*/

extern int optind, optreset;
static void checkPrefs( struct SyncPrefs expectedPrefs, char* cmdLine);

void testEmptyCmdLine(void** state){
    struct SyncPrefs prefs = {0, 0, NULL, 0, 0, NULL, ERR_OPT_NO_ARGS};
    checkPrefs(prefs, "");
}

void testPort(void** state){
    struct SyncPrefs prefsBad = {0, 0, NULL, 0, 0, NULL, ERR_BAD_PORT};

    checkPrefs(prefsBad, "-p 0 h1");
    checkPrefs(prefsBad, "-p 65536 h1");
    checkPrefs(prefsBad, "-p bork h1");
    
    char *hosts[1];
    hosts[0] = "h1";
    struct SyncPrefs prefsOk1 = {0, 0, hosts, 1, 1, NULL, NULL};
    checkPrefs(prefsOk1, "-p 1 h1");
    
    struct SyncPrefs prefsOk2 = {0, 0, hosts, 1, 65535, NULL, NULL};
    checkPrefs(prefsOk2, "-p 65535 h1");
    
    struct SyncPrefs prefsOk3 = {0, 0, hosts, 1, 80, NULL, NULL};
    checkPrefs(prefsOk3, "-p80 h1");
}

void testVersion(void** state){
    struct SyncPrefs prefs = {1, 0, NULL, 0, 0, NULL, NULL};
    checkPrefs(prefs, "-v");
}

void testHelp(void** state){
    struct SyncPrefs prefs = {0, 1, NULL, 0, 0, NULL, NULL};
    checkPrefs(prefs, "-h");
}

void testHost(void** state){
    char* host1[1];
    host1[0] = strdup("h1");

    struct SyncPrefs prefs1 = {0, 0, host1, 1, 0, NULL, NULL};
    checkPrefs(prefs1, "h1");
    free(host1[0]);
    
    char* host2[3];
    host2[0] = strdup("h1");
    host2[1] = strdup("h2");
    host2[2] = strdup("h3");

    struct SyncPrefs prefs2 = {0, 0, host2, 3, 0, NULL, NULL};
    checkPrefs(prefs2, "h1 h2 h3");
    free(host2[0]);
    free(host2[1]);
    free(host2[2]);
}

void testAlias(void** state){
    char* host1[2];
    host1[0] = strdup("h1");
    host1[1] = strdup("h2");

    struct SyncPrefs prefs1 = {0, 0, host1, 0, 0, "alias", ERR_MULTIPLE_HOSTS_ONE_ALIAS};
    checkPrefs(prefs1, "-a alias h1 h2");
    free(host1[0]);
    free(host1[1]);

    char* host2[1];
    host2[0] = strdup("h1");

    struct SyncPrefs prefs2 = {0, 0, host2, 1, 0, "alias", NULL};
    checkPrefs(prefs2, "-a alias h1");
    free(host2[0]);
}

void testVariousValid(void** state){
    char* host1[1];
    host1[0] = strdup("myhost");

    struct SyncPrefs prefs1 = {0, 0, host1, 1, 8080, "myalias", NULL};
    checkPrefs(prefs1, "-p 8080 -a myalias myhost");
    free(host1[0]);

    char* host2[3];
    host2[0] = strdup("h1");
    host2[1] = strdup("h2");
    host2[2] = strdup("h3");

    struct SyncPrefs prefs2 = {0, 0, host2, 3, 10, NULL, NULL};
    checkPrefs(prefs2, "-p10 h1 h2 h3");
    free(host2[0]);
    free(host2[1]);
    free(host2[2]);
}

static void checkPrefs(struct SyncPrefs expectedPrefs, char* cmdLine){
 // Helper function for checking the values in a Prefs structure
    char** argv;
    int argc;
    parseCommandLine(cmdLine, &argv, &argc);
    
    struct SyncPrefs actualPrefs = {0, 0, NULL, 0, 0, NULL, NULL};
    optind = 1; // need to reset this global between each call to getopt()
    #if HAVE_DECL_OPTRESET
        optreset = 1; // non-portable way to reset state - see getopt.h
    #endif

    parseSyncArgs(argc, argv, &actualPrefs);

    assert_int_equal(expectedPrefs.help,      actualPrefs.help);
    assert_int_equal(expectedPrefs.version,   actualPrefs.version);
    assert_int_equal(expectedPrefs.hostCount, actualPrefs.hostCount);
    assert_int_equal(expectedPrefs.port,      actualPrefs.port);

    int i;
    for (i=0; i<expectedPrefs.hostCount; i++){
        assert_string_equal(expectedPrefs.hosts[i], actualPrefs.hosts[i]);
        free(actualPrefs.hosts[i]);
    }
    if (actualPrefs.hosts != NULL){
        free(actualPrefs.hosts);
    }

    if (expectedPrefs.alias == NULL){
        assert_true(actualPrefs.alias == NULL);
    } else {
        assert_string_equal(expectedPrefs.alias, actualPrefs.alias);
        free(actualPrefs.alias);
    }

    if (expectedPrefs.errMsg == NULL){
        assert_true(actualPrefs.errMsg == NULL);
    } else {
        assert_string_equal(expectedPrefs.errMsg, actualPrefs.errMsg);
        free(actualPrefs.errMsg);
    }
    
    int j=0;
    while(j<argc){
        free(argv[j++]);
    }
    free(argv);
}
