#include "common.h"
#include <time.h>

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
        SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
        PathAppend(path, TEXT(OUT_DIR));
        PathAppend(path, TEXT(DB_NAME));
    }

    static void getLogPath(char* path){
        SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
        PathAppend(path, TEXT(OUT_DIR));
        PathAppend(path, TEXT(LOG_NAME));
    }
#endif

#ifdef __linux__
    #include <string.h>
    #include <unistd.h>

    void doSleep(int interval){
        sleep(interval);
    }


    void getDbPath(char* path){
        strcpy(path, "/var/lib/bitmeter/" DB_NAME);
    }

    void getLogPath(char* path){
        strcpy(path, "/var/log/bitmeter/error.log");
    }
#endif

#ifdef __APPLE__
    #include <string.h>

    void doSleep(int interval){
        sleep(interval);
    }

    void getDbPath(char* path){
        strcpy(path, "/Library/Application Support/BitMeter/" DB_NAME);
    }

    void getLogPath(char* path){
        strcpy(path, "/Library/Logs/bitmeter.log");
    }
#endif
