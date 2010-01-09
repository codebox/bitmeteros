/*
 * BitMeterOS v0.3.0
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2009 Rob Dawson
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
 * Build Date: Sat, 09 Jan 2010 16:37:16 +0000
 */

#include <stdio.h>

#define ERR_OPT_NO_ARGS "No arguments were supplied"
#define ERR_BAD_ACTION  "Unrecognised action"
#define ERR_BAD_UPGRADE_LEVEL "Invalid upgrade level specified"
#define INFO_DUMPING_CONFIG "Dumping config table..."

#define MAX_UPGRADE_LEVEL 3

int doVersion();
int convertAddrValues();
int doConfig(FILE* file, int argc, char** argv);
int doUpgrade(FILE* file, int argc, char** argv);
int setConfigIntValue(char* key, int value);
