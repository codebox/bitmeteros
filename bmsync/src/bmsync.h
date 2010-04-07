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

#define OPT_HELP    'h'
#define OPT_VERSION 'v'
#define OPT_PORT    'p'
#define OPT_ALIAS   'a'

#define HTTP_EOL "\r\n"
#define ERR_OPT_NO_ARGS "No arguments were supplied"
#define ERR_NO_HOST "No host name/IP address was supplied"
#define ERR_MULTIPLE_HOSTS_ONE_ALIAS "Multiple hosts were specified with a single alias, this probably isn't what you want to do"
#define ERR_BAD_PORT "Bad port number"

#define SYNC_NAME "bmsync"

struct SyncPrefs{
	int version;
	int help;
	char** hosts;
	int hostCount;
	int port;
	char* alias;
	char* errMsg;
};

int parseSyncArgs(int argc, char **argv, struct SyncPrefs *prefs);
void doHelp();
void doVersion();
