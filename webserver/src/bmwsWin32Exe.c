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

#include <signal.h>
#include "common.h"
#include "bmws.h"

static void sigHandler();

int main(){
	setupWeb();	
	signal(SIGINT, sigHandler);	// Trap Ctrl-C
	printf(COPYRIGHT);
	printf("Web Server running, press Ctrl-C to quit..." EOL);
	
    while (TRUE){
        processWeb();
    }

    return 0;
}

static void sigHandler(){
	shutdownWeb();
	printf("Web Server stopped." EOL);
	exit(0);
}
