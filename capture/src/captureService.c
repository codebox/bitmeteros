/*
 * BitMeterOS v0.3.2
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
 *
 * Build Date: Sun, 07 Mar 2010 14:49:47 +0000
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "capture.h"
#include "common.h"

/*
Contains the entry point for the data capture application, when run as a Windows Service.
*/

SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   hStatus; 

void ServiceMain(int argc, char** argv); 
void ControlHandler(DWORD request); 

int main(){
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = SERVICE_NAME;
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	StartServiceCtrlDispatcher(ServiceTable);  
}

void ServiceMain(int argc, char** argv) { 
	ServiceStatus.dwServiceType             = SERVICE_WIN32; 
	ServiceStatus.dwCurrentState            = SERVICE_START_PENDING; 
	ServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode           = 0; 
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	ServiceStatus.dwCheckPoint              = 0; 
	ServiceStatus.dwWaitHint                = 0; 
 
 	setLogLevel(LOG_ERR);
 	setLogToFile(TRUE);
	setupCapture();
	
	hStatus = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)ControlHandler); 
	if (hStatus == (SERVICE_STATUS_HANDLE)0) { 
		logMsg(LOG_ERR, "Failed to register service control handle");
		return; 
	}  

 // We report the running status to SCM. 
	ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
	SetServiceStatus (hStatus, &ServiceStatus);
   	
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
   		doSleep(1);
   		processCapture();
	}
	shutdownCapture();
	
	return;
}

void ControlHandler(DWORD request) { 
 // Requests for a change of state arrive here
	if ((request == SERVICE_CONTROL_STOP) || (request == SERVICE_CONTROL_SHUTDOWN)){
		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
	}
 
 // Report current status
    SetServiceStatus (hStatus, &ServiceStatus);
}
