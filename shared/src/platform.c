/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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
 */

#include "common.h"
#include <time.h>
#include <stdlib.h>

/*
Contains functions that must be replaced according to OS/platform, and according
to whether we are running tests or not. When we unit test, this file is completely
replaced by another containing test-friendly implementations of these functions.
*/
time_t getTime(){
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
         /* If the 'BITMETER_DB' environment variable hasn't been set then we expect the
            database to exist in the 'All Users\Application Data\BitMeterOS folder. */
            SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
            PathAppend(path, TEXT(OUT_DIR));
            PathAppend(path, TEXT(DB_NAME));
        } else {
         // The 'BITMETER_DB' environment variable is set, so use that location instead.
            strcpy(path, envValue);
        }
    }


    void getLogPath(char* path){
        char* configValue;
        if ((isDbOpen() == FALSE) || (configValue = getConfigText(CONFIG_LOG_PATH, TRUE)) == NULL){
         /* If an alternative path has not been specified in the db config table, or if the
            database isn't open yet, then we write the log out to the 'All Users\Application Data\BitMeterOS'
            folder. */
            SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
            PathAppend(path, TEXT(OUT_DIR));
            PathAppend(path, TEXT(LOG_NAME));
        } else {
         // A value was supplied in the config table, so use that location instead.
            strcpy(path, configValue);
        }
    }

    void getWebRootPath(char* path){
        char* configValue;
        if ((configValue = getConfigText(CONFIG_WEB_DIR, TRUE)) == NULL){
         /* If an alternative path has not been specified in the db config table then we
            use the 'All Users\Application Data\BitMeterOS' folder as the web server root. */
            SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path);
            PathAppend(path, TEXT(OUT_DIR));
            PathAppend(path, "web");

        } else {
         // A value was supplied in the config table, so use that location instead.
            strcpy(path, configValue);
        }

     // Need to ensure we end on a folder delimiter character, so we can prevent directory-traversal attacks
        int pathLen = strlen(path);
        if (path[pathLen-1] != '\\'){
        	path[pathLen] = '\\';
        	path[pathLen + 1] = (char) 0;
        }
    }

	void logWin32ErrMsg(char* msg, int rc) {
	    LPVOID lpMsgBuf;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	            NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
		logMsg(LOG_ERR, "%s. Code=%d Msg=%s", msg, rc, lpMsgBuf);
	    LocalFree(lpMsgBuf);
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
         /* If the 'BITMETER_DB' environment variable hasn't been set then we expect the
            database to exist in the /var/lib/bitmeter directory. */
            strcpy(path, "/var/lib/bitmeter/" DB_NAME);
        } else {
         // The 'BITMETER_DB' environment variable is set, so use that location instead.
            strcpy(path, envValue);
        }
    }

    void getLogPath(char* path){
        char* configValue;
        if ((isDbOpen() == FALSE) || (configValue = getConfigText(CONFIG_LOG_PATH, TRUE)) == NULL){
         /* If an alternative path has not been specified in the db config table, or if the
            database isn't open yet, then we write the log out to the /var/log/bitmeter directory. */
            strcpy(path, "/var/log/bitmeter/error.log");
        } else {
         // A value was supplied in the config table, so use that location instead.
            strcpy(path, configValue);
        }
    }

    void getWebRootPath(char* path){
        char* configValue;
        if ((configValue = getConfigText(CONFIG_WEB_DIR, TRUE)) == NULL){
         /* If an alternative path has not been specified in the db config table then we
            use the /var/www/bitmeter directory as the web server root. */
            strcpy(path, "/var/www/bitmeter/");
        } else {
         // A value was supplied in the config table, so use that location instead.
            strcpy(path, configValue);
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
         /* If the 'BITMETER_DB' environment variable hasn't been set then we expect the
            database to exist in the /Library/Application Support/BitMeter directory. */
            strcpy(path, "/Library/Application Support/BitMeter/" DB_NAME);
        } else {
         // The 'BITMETER_DB' environment variable is set, so use that location instead.
            strcpy(path, envValue);
        }
    }

    void getLogPath(char* path){
        char* configValue;
        if ((isDbOpen() == FALSE) || (configValue = getConfigText(CONFIG_LOG_PATH, TRUE)) == NULL){
         /* If an alternative path has not been specified in the db config table, or if the
            database isn't open yet, then we write the log out to the /Library/Logs directory. */
            strcpy(path, "/Library/Logs/bitmeter.log");
        } else {
         // A value was supplied in the config table, so use that location instead.
            strcpy(path, configValue);
        }
    }

    void getWebRootPath(char* path){
        char* configValue;
        if ((configValue = getConfigText(CONFIG_WEB_DIR, TRUE)) == NULL){
         /* If an alternative path has not been specified in the db config table then we
            use the /Library/Application Support/BitMeter/www directory as the web server root. */
            strcpy(path, "/Library/Application Support/BitMeter/www/");
        } else {
         // A value was supplied in the config table, so use that location instead.
            strcpy(path, configValue);
        }
    }
#endif
