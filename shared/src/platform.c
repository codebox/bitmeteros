#include "common.h"
#include <time.h>
#include <stdlib.h>

int getTime(){
	return time(NULL);
}

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <shlwapi.h>

    void doSleep(int interval){
        Sleep(interval * 1000);
    }

    void getDbPath(char* path){
        char* envValue;
        if ((envValue = getenv(ENV_DB)) == NULL){
            SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
            PathAppend(path, TEXT(OUT_DIR));
            PathAppend(path, TEXT(DB_NAME));
        } else {
            strcpy(path, envValue);
        }
    }


    void getLogPath(char* path){char* envValue;
        if ((envValue = getenv(ENV_LOG)) == NULL){
            SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
            PathAppend(path, TEXT(OUT_DIR));
            PathAppend(path, TEXT(LOG_NAME));
        } else {
            strcpy(path, envValue);
        }
    }

#endif

#ifdef __linux__
    #include <string.h>
    #include <unistd.h>

    void doSleep(int interval){
        sleep(interval);
    }


    void getDbPath(char* path){
        char* envValue;
        if ((envValue = getenv(ENV_DB)) == NULL){
            strcpy(path, "/var/lib/bitmeter/" DB_NAME);
        } else {
            strcpy(path, envValue);
        }
    }

    void getLogPath(char* path){
        char* envValue;
        if ((envValue = getenv(ENV_LOG)) == NULL){
            strcpy(path, "/var/log/bitmeter/error.log");
        } else {
            strcpy(path, envValue);
        }
    }
#endif

#ifdef __APPLE__
    #include <string.h>

    void doSleep(int interval){
        sleep(interval);
    }

    void getDbPath(char* path){
        char* envValue;
        if ((envValue = getenv(ENV_DB)) == NULL){
            strcpy(path, "/Library/Application Support/BitMeter/" DB_NAME);
        } else {
            strcpy(path, envValue);
        }
    }

    void getLogPath(char* path){
        char* envValue;
        if ((envValue = getenv(ENV_LOG)) == NULL){
            strcpy(path, "/Library/Logs/bitmeter.log");
        } else {
            strcpy(path, envValue);
        }
    }
#endif
