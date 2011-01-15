/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
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
#include <unistd.h>
#include <stdio.h>

// Executes commands used to stop/start the data capture and web server processes

#ifdef _WIN32
	#include <windows.h>
	#include <winbase.h>
	#define BMWS_SERVICE_NAME  "BitMeterWebService"
	#define BMCAP_SERVICE_NAME "BitMeterCaptureService"

	int runCmd(char* cmd){
		PROCESS_INFORMATION pi;
	    STARTUPINFO si;
	    memset(&si,0,sizeof(si));
	    si.cb= sizeof(si);

	    if (CreateProcess(0, cmd, 0, 0, FALSE, 0, 0, 0, &si, &pi)){
	    	WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return SUCCESS;
			
	    } else {
	    	printf("Error, failed to execute command: %s\n", cmd );
	    	return FAIL;
	    }

	}

	int doWebStop(FILE* file, int argc, char** argv){
		return runCmd("sc stop " BMWS_SERVICE_NAME);
	}

	int doWebStart(FILE* file, int argc, char** argv){
		return runCmd("sc start " BMWS_SERVICE_NAME);
	}

	int doCapStop(FILE* file, int argc, char** argv){
		return runCmd("sc stop " BMCAP_SERVICE_NAME);
	}

	int doCapStart(FILE* file, int argc, char** argv){
		return runCmd("sc start " BMCAP_SERVICE_NAME);
	}
#endif

#if defined(__linux__) || defined(__APPLE__)
	int runCmd(const char *args[]){
		if (geteuid() == 0) {
			int pid = fork();
			int status;

			if (pid == 0){
				execvp(args[0], args);
				_exit(1);
				return SUCCESS; // TODO correct?
				
			} else if (pid < 0){
				fprintf(stderr, "Failed to execute command\n");
				return FAIL;
				
			} else {
				wait(&status);
				return SUCCESS; // TODO correct?   
			}
			
		} else {
			fprintf(stderr, "You need to run this command as root (try using 'sudo')\n");
			return FAIL;
		}
	}

#endif

#ifdef __linux__
	#include <sys/types.h>
	#define BMWS_INIT_SCRIPT  "/etc/init.d/bitmeterweb"
	#define BMCAP_INIT_SCRIPT "/etc/init.d/bitmeter"

	int doWebStop(FILE* file, int argc, char** argv){
		const char *args[] = {BMWS_INIT_SCRIPT, "stop", 0};
		return runCmd(args);
	}

	int doWebStart(FILE* file, int argc, char** argv){
		const char *args[] = {BMWS_INIT_SCRIPT, "start", 0};
		return runCmd(args);
	}

	int doCapStop(FILE* file, int argc, char** argv){
		const char *args[] = {BMCAP_INIT_SCRIPT, "stop", 0};
		return runCmd(args);
	}

	int doCapStart(FILE* file, int argc, char** argv){
		const char *args[] = {BMCAP_INIT_SCRIPT, "start", 0};
		return runCmd(args);
	}
#endif

#ifdef __APPLE__
	#define BMWS_UID  "uk.org.codebox.bitmeterweb"
	#define BMCAP_UID "uk.org.codebox.bitmeter"

	int doWebStop(FILE* file, int argc, char** argv){
		const char *args[] = {"launchctl", "stop", BMWS_UID, 0};
		return runCmd(args);
	}

	int doWebStart(FILE* file, int argc, char** argv){
		const char *args[] = {"launchctl", "start", BMWS_UID, 0};
		return runCmd(args);
	}

	int doCapStop(FILE* file, int argc, char** argv){
		const char *args[] = {"launchctl", "stop", BMCAP_UID, 0};
		return runCmd(args);
	}

	int doCapStart(FILE* file, int argc, char** argv){
		const char *args[] = {"launchctl", "start", BMCAP_UID, 0};
		return runCmd(args);
	}
#endif
